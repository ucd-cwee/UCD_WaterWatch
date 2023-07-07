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
#include "WaterWatch_Module_EPAnet.h"

#include "EPAnetWrapper.h" 
#include "cweeUnitedValue.h"
#include "enum.h"
#include "cweeUnitPattern.h"
#include "fileSystemH.h"
#include "cweeScheduler.h"
#include "InterpolatedMatrix.h"
#include "Engineering.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_EPAnet() {
            auto lib = chaiscript::make_shared<Module>();
#if 1
            // EPAnet 2.2 Wrapper
#if 1
            if (1) {
                /* Spattern */ {
                    AddSharedPtrClass(::epanet, Spattern);
                    AddSharedPtrClassMember(::epanet, Spattern, Comment);
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Spattern> const& a) { if (a) return cweeUnitValues::cweeUnitPattern(a->Pat); else return cweeUnitValues::cweeUnitPattern(); }), "Pat");
                }
                /* Scurve */ {
                    AddSharedPtrClass(::epanet, Scurve);
                    AddSharedPtrClassMember(::epanet, Scurve, Comment);
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Scurve> const& a) { if (a) return cweeUnitValues::cweeUnitPattern(a->Curve); else return cweeUnitValues::cweeUnitPattern(); }), "Pat");
                }
                /* Sdemand */ {
                    AddSharedPtrClass(::epanet, Sdemand);
                    AddSharedPtrClassMember(::epanet, Sdemand, Base);
                    AddSharedPtrClassMember(::epanet, Sdemand, Pat);
                    AddSharedPtrClassMember(::epanet, Sdemand, TimePat);
                }
                /* Senergy */ {
                    AddSharedPtrClass(::epanet, Senergy);
                    AddSharedPtrClassMember(::epanet, Senergy, TimeOnLine);
                    AddSharedPtrClassMember(::epanet, Senergy, Efficiency);
                    AddSharedPtrClassMember(::epanet, Senergy, KwHrsPerFlow);
                    AddSharedPtrClassMember(::epanet, Senergy, KwHrs);
                    AddSharedPtrClassMember(::epanet, Senergy, MaxKwatts);
                    AddSharedPtrClassMember(::epanet, Senergy, TotalCost);
                    AddSharedPtrClassMember(::epanet, Senergy, CurrentPower);
                    AddSharedPtrClassMember(::epanet, Senergy, CurrentEffic);
                }
                /* Ssource */ {
                    AddSharedPtrClass(::epanet, Ssource);
                    AddSharedPtrClassMember(::epanet, Ssource, Concentration);
                    AddSharedPtrClassMember(::epanet, Ssource, Pat);
                    AddSharedPtrClassMember(::epanet, Ssource, TimePat);
                    AddSharedPtrClassMember(::epanet, Ssource, Smass);
                }
                /* Svertices */ {
                    AddSharedPtrClass(::epanet, Svertices);
                    AddSharedPtrClassMember(::epanet, Svertices, Array); // List<pair<SCALAR, SCALAR>>
                }
                /* Sasset */ {
                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(asset_t, asset_t);
                    AddSharedPtrClass(::epanet, Sasset);
                    AddSharedPtrClassFunction(::epanet, Sasset, Icon);
                    AddSharedPtrClassMember(::epanet, Sasset, Type_p);
                    AddSharedPtrClassMember(::epanet, Sasset, Name_p);

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_HEAD_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_HEAD_>::unit)(1)); }), "Head");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_DEMAND_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_DEMAND_>::unit)(1)); }), "Demand");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_FLOW_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_FLOW_>::unit)(1)); }), "Flow");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_ENERGY_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_ENERGY_>::unit)(1)); }), "Energy");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_HEADLOSS_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_HEADLOSS_>::unit)(1)); }), "Headloss");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_VELOCITY_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_VELOCITY_>::unit)(1)); }), "Velocity");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_SETTING_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_SETTING_>::unit)(1)); }), "Setting");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_STATUS_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_STATUS_>::unit)(1)); }), "Status");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Sasset> const& a) { AUTO p = a->GetValue<_QUALITY_>(); if (p) return cweeUnitValues::cweeUnitPattern(*p); else return cweeUnitValues::cweeUnitPattern((units::time::second_t)(1), (typename ::epanet::DefaultUnits<_QUALITY_>::unit)(1)); }), "Quality");
                }
                /* Snode */ {
                    AddSharedPtrClass(::epanet, Snode);
                    AddSharedPtrClassMember(::epanet, Snode, X);
                    AddSharedPtrClassMember(::epanet, Snode, Y);
                    AddSharedPtrClassMember(::epanet, Snode, El);
                    AddSharedPtrClassMember(::epanet, Snode, D);
                    AddSharedPtrClassMember(::epanet, Snode, S);
                    AddSharedPtrClassMember(::epanet, Snode, Ke);
                    AddSharedPtrClassMember(::epanet, Snode, Zone);
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");

                    AddSharedPtrClassFunction(::epanet, Snode, HasWaterDemand);
                    AddSharedPtrClassFunction(::epanet, Snode, GetMinPressure);
                    AddSharedPtrClassFunction(::epanet, Snode, GetMaxPressure);
                    AddSharedPtrClassFunction(::epanet, Snode, GetAvgPressure);

                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Snode>>());
                }
                /* Slink */ {
                    AddSharedPtrClass(::epanet, Slink);
                    AddSharedPtrClassMember(::epanet, Slink, StartingNode);
                    AddSharedPtrClassMember(::epanet, Slink, EndingNode);
                    AddSharedPtrClassMember(::epanet, Slink, Diam);
                    AddSharedPtrClassMember(::epanet, Slink, Len);
                    AddSharedPtrClassMember(::epanet, Slink, Kc);
                    AddSharedPtrClassMember(::epanet, Slink, Km);
                    AddSharedPtrClassMember(::epanet, Slink, Kb);
                    AddSharedPtrClassMember(::epanet, Slink, Kw);
                    AddSharedPtrClassMember(::epanet, Slink, R_FlowResistance);
                    AddSharedPtrClassMember(::epanet, Slink, Rc);
                    AddSharedPtrClassMember(::epanet, Slink, Vertices);
                    AddSharedPtrClassFunction(::epanet, Slink, Area);
                    AddSharedPtrClassFunction(::epanet, Slink, IsBiDirectionalPipe);
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Slink>>());
                }
                /* Stank */ {
                    AddSharedPtrClass(::epanet, Stank);
                    AddSharedPtrClassMember(::epanet, Stank, Node);
                    AddSharedPtrClassMember(::epanet, Stank, Diameter);                    
                    AddSharedPtrClassMember(::epanet, Stank, Hmin);
                    AddSharedPtrClassMember(::epanet, Stank, Hmax);
                    AddSharedPtrClassMember(::epanet, Stank, Kb);
                    AddSharedPtrClassMember(::epanet, Stank, TimePat);
                    AddSharedPtrClassMember(::epanet, Stank, Pat);
                    AddSharedPtrClassMember(::epanet, Stank, Vcurve);
                    AddSharedPtrClassMember(::epanet, Stank, Vcurve_Actual);
                    AddSharedPtrClassMember(::epanet, Stank, MixModel);
                    AddSharedPtrClassMember(::epanet, Stank, V1frac);
                    AddSharedPtrClassMember(::epanet, Stank, CanOverflow);
                    AddSharedPtrClassFunction(::epanet, Stank, Area);
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Stank> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Snode>, cweeSharedPtr<::epanet::Stank>>());
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Stank>>());
                }
                /* Spump */ {
                    AddSharedPtrClass(::epanet, Spump);
                    AddSharedPtrClassMember(::epanet, Spump, Link);
                    AddSharedPtrClassMember(::epanet, Spump, Ptype);
                    AddSharedPtrClassMember(::epanet, Spump, Qmax);
                    AddSharedPtrClassMember(::epanet, Spump, Hmax);
                    AddSharedPtrClassMember(::epanet, Spump, H0);
                    AddSharedPtrClassMember(::epanet, Spump, R);
                    AddSharedPtrClassMember(::epanet, Spump, N);
                    AddSharedPtrClassMember(::epanet, Spump, Hcurve);
                    AddSharedPtrClassMember(::epanet, Spump, Ecurve);
                    AddSharedPtrClassMember(::epanet, Spump, TimeUpat);
                    AddSharedPtrClassMember(::epanet, Spump, TimeEpat);
                    AddSharedPtrClassMember_SpecializedName(::epanet, Spump, Energy, EnergySummary);
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Slink>, cweeSharedPtr<::epanet::Spump>>());
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Spump>>());
                }
                /* Svalve */ {
                    AddSharedPtrClass(::epanet, Svalve);
                    AddSharedPtrClassMember(::epanet, Svalve, Link);
                    AddSharedPtrClassMember(::epanet, Svalve, ProducesElectricity);
                    AddSharedPtrClassMember_SpecializedName(::epanet, Svalve, Energy, EnergySummary);
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");

                    //lib->AddFunction(, NetPresentValue, , SINGLE_ARG({
                    //    return proj->epanetProj->network->Leakage.NetPresentValueOfValve(valve);
                    //}), ::epanet::Pvalve const& valve, cweeSharedPtr<EPAnetProject> const& proj);

                    //lib->AddFunction(, NetPresentValue, , SINGLE_ARG({
                    //    return proj->epanetProj->network->Leakage.NetPresentValueOfValve(valve, mode);
                    //}), ::epanet::Pvalve const& valve, cweeSharedPtr<EPAnetProject> const& proj, int mode);

                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Slink>, cweeSharedPtr<::epanet::Svalve>>());
                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Svalve>>());
                }
                /* Szone */ {
                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::zoneType_t, zoneType_t);
                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::illDefined_t, illDefined_t);
                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::direction_t, direction_t);

                    AddSharedPtrClass(::epanet, Szone);                    

                    AddSharedPtrClassMember(::epanet, Szone, IllDefined);
                    AddSharedPtrClassMember(::epanet, Szone, Node);
                    AddSharedPtrClassMember(::epanet, Szone, Within_Link);
                    AddSharedPtrClassMember(::epanet, Szone, Boundary_Link);   

                    AddSharedPtrClassFunction(::epanet, Szone, AverageElevation);
                    AddSharedPtrClassFunction(::epanet, Szone, HasWaterDemand);
                    AddSharedPtrClassFunction(::epanet, Szone, IsIllDefined);
                    AddSharedPtrClassFunction(::epanet, Szone, AverageNodePressure);
                    AddSharedPtrClassFunction(::epanet, Szone, MinimumNodePressure);
                    AddSharedPtrClassFunction(::epanet, Szone, AverageCustomerPressure);
                    AddSharedPtrClassFunction(::epanet, Szone, MinimumCustomerPressure);

                    AddSharedPtrClassFunction(::epanet, Szone, FindMinPressureCustomer);
                    AddSharedPtrClassFunction(::epanet, Szone, FindMinPressureNode);

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a) -> std::string { if (!a) throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); return (cweeStr(a->Type_p.ToString()) + " " + a->Name_p).c_str(); }), "to_string");

                    lib->add(chaiscript::fun([](::epanet::Pzone const& a, cweeSharedPtr<EPAnetProject>& proj) {
                        // std::vector< chaiscript::Boxed_Value >;

                        std::vector<chaiscript::Boxed_Value> out; // std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value> // object, ::epanet::direction_t
                        
                        AUTO assets = a->GetMassBalanceAssets(proj->epanetProj);

                        for (auto& x : assets) {
                            epanet::Passet assetP = x.get<0>();
                            epanet::direction_t dir = x.get<1>();

                            if (assetP) {
                                switch (assetP->Type_p) {
                                case asset_t::JUNCTION:
                                    out.emplace_back(var(std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var(assetP.CastReference<epanet::Snode>()), var(std::move(dir)))));
                                    break;
                                case asset_t::RESERVOIR:
                                    out.emplace_back(var(std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var(assetP.CastReference<epanet::Stank>()), var(std::move(dir)))));
                                    break;
                                case asset_t::PIPE:
                                    out.emplace_back(var(std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var(assetP.CastReference<epanet::Slink>()), var(std::move(dir)))));
                                    break;
                                case asset_t::PUMP:
                                    out.emplace_back(var(std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var(assetP.CastReference<epanet::Spump>()), var(std::move(dir)))));
                                    break;
                                case asset_t::VALVE:
                                    out.emplace_back(var(std::pair<chaiscript::Boxed_Value, chaiscript::Boxed_Value>(var(assetP.CastReference<epanet::Svalve>()), var(std::move(dir)))));
                                    break;
                                }
                            }
                        }

                        return out;
                    }), "GetMassBalanceAssets");
                    lib->add(chaiscript::fun([](::epanet::Pzone& a, cweeSharedPtr<EPAnetProject>& proj, std::map<std::string, Boxed_Value> const& scadaPatterns) {
                        AUTO set = std::map<std::string, cweeUnitValues::cweeUnitPattern>();
                        for (auto& x : scadaPatterns) {
                            AUTO ptr = boxed_cast<cweeUnitValues::cweeUnitPattern*>(x.second);
                            if (ptr) {
                                set[x.first] = *ptr;
                            }
                        }
                        return a->TryCalibrateZone(proj->epanetProj, set);
                    }), "TryCalibrateZone");
                    lib->AddFunction(, LeakModelResults, , SINGLE_ARG({
                        std::map<std::string, Boxed_Value> out;
                        for (auto& item : zone->LeakModelResults(proj->epanetProj, surveyFrequency, oldPressure)) out[item.first] = var(cweeUnitValues::unit_value(item.second));
                        return out;
                    }), ::epanet::Pzone const& zone, cweeSharedPtr<EPAnetProject> const& proj, units::time::year_t surveyFrequency, units::pressure::pounds_per_square_inch_t oldPressure);
                    lib->AddFunction(, SurveyFrequency, , SINGLE_ARG({
                        return zone->SurveyFrequency(proj->epanetProj, oldPressure);
                    }), ::epanet::Pzone const& zone, cweeSharedPtr<EPAnetProject> const& proj, units::pressure::pounds_per_square_inch_t oldPressure);

                    lib->add(chaiscript::castable_class<cweeSharedPtr<::epanet::Sasset>, cweeSharedPtr<::epanet::Szone>>());
                }

                // ...

                /* Network */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Network);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nnodes);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ntanks);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Njuncs);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nlinks);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npipes);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npumps);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nvalves);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ncontrols);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nrules);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npats);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ncurves);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, System);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Asset);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Node);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Link);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Tank);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Pump);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Valve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Zone);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Pattern);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Curve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Control);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Rule);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Adjlist);

                    AddSharedPtrClassFunction(::epanet, Network, getCalibrationOrder);
                }
                /* Project */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Project);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, network);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, parser);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, times);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, report);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, outfile);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, hydraul);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, quality);
                }

                /* EPAnet Wrapper */ {
                    {
                        lib->add(chaiscript::user_type<EPAnet_Shared>(), "EPAnet");
                        lib->add(chaiscript::fun([]()->EPAnet_Shared* { return &*EPAnet; }), "EPAnet");
                        lib->add(chaiscript::fun([](EPAnet_Shared& p, cweeStr filePath)->cweeSharedPtr<EPAnetProject> { AUTO proj = p.createNewProject(); proj->loadINP(filePath); return proj; }), "loadProject");
                        lib->add(chaiscript::fun([](EPAnet_Shared& p)->cweeSharedPtr<EPAnetProject> { return p.createNewProject(); }), "createNewProject");
                        lib->add(chaiscript::fun([](EPAnet_Shared& p, cweeSharedPtr<EPAnetProject>& proj) -> cweeSharedPtr<EPAnetProject> {
                            auto fp = fileSystem->createRandomFile(fileType_t::INP);
                            proj->saveINP(fp); // save to a temp file
                            auto out = p.createNewProject();
                            out->loadINP(fp);
                            return out;
                            }), "copyProject");
                    }
                    {
                        lib->add(chaiscript::user_type<cweeSharedPtr<EPAnetProject>>(), "EPAnetProject");
                        lib->add(fun([]() { return EPAnet->createNewProject(); }), "EPAnetProject");
                        lib->add(constructor<cweeSharedPtr<EPAnetProject>(const cweeSharedPtr<EPAnetProject>&)>(), "EPAnetProject");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& a, const cweeSharedPtr<EPAnetProject>& b) { a = b; return a; }), "=");

                        lib->add(fun([](cweeSharedPtr<EPAnetProject> const& a) { return &*a->epanetProj; }), "project"); // data member as function, accessible as member or function in script. (i.e: "auto proj = EPAnet.createNewProject(); return proj.project.network.Nnodes;")
                        lib->add(fun([](cweeSharedPtr<EPAnetProject> const& a)-> cweeSharedPtr<epanet::Network>&{ return a->epanetProj->network; }), "network"); // easier access to underlying network


                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->initBuild("", "", 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile) { proj->initBuild(rptFile, "", 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile) { proj->initBuild(rptFile, outFile, 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile, int unitsType) { proj->initBuild(rptFile, outFile, unitsType, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile, int unitsType, int headLossType) { proj->initBuild(rptFile, outFile, unitsType, headLossType); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile, cweeStr filePathToRptFile) { proj->loadINP(filePathToInpFile, filePathToRptFile); }), "loadINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile) { proj->loadINP(filePathToInpFile); }), "loadINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->closeINP(); }), "closeINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile) { proj->saveINP(filePathToInpFile); }), "saveINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getTitle(); }), "getTitle");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, bool isNode, int epanetIndex) { return proj->getDescription(isNode, epanetIndex); }), "getDescription");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, bool isNode, int epanetIndex, cweeStr description) { proj->setDescription(isNode, epanetIndex, description); }), "setDescription");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int objectType) { return proj->getCount(objectType); }), "getCount");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int units) { proj->setFlowUnits(units); }), "setFlowUnits");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int qualType, cweeStr chemName, cweeStr chemUnits, cweeStr traceNode) { proj->setQualType(qualType, chemName, chemUnits, traceNode); }), "setQualType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int param, units::time::second_t value) { proj->setTimeParam(param, value); }), "setTimeParam");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int option, double value) { proj->setOption(option, value); }), "setOption");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int option) { return proj->getOption(option); }), "getOption");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int param) { return proj->getTimeParam(param); }), "getTimeParam");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getFlowUnits(); }), "getFlowUnits");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentSimTime(); }), "getCurrentSimTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentSimTimestep(); }), "getCurrentSimTimestep");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentError(); }), "getCurrentError");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->fullHydraulicSim(); }), "fullHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->startHydraulicSim(); }), "startHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcHydraulicsAtTime(); }), "calcHydraulicsAtTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcHydraulicTimestepToNextTime(); }), "calcHydraulicTimestepToNextTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->resetHydraulicSim(); }), "resetHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->endHydraulicSim(); }), "endHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->fullQualitySim(); }), "fullQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->startQualitySim(); }), "startQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcQualityAtTime(); }), "calcQualityAtTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcQualityTimestepToNextTime(); }), "calcQualityTimestepToNextTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->resetQualitySim(); }), "resetQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->endQualitySim(); }), "endQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, int nodeType) { return proj->addNode(name, nodeType); }), "addNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int actionCode) { proj->deleteNode(nodeIndex, actionCode); }), "deleteNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, cweeStr newid) { proj->setNodeID(nodeIndex, newid); }), "setNodeID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int property, double value) { proj->setNodeValue(nodeIndex, property, value); }), "setNodeValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double elev, double dmnd, cweeStr dmndpat) { proj->setJuncData(nodeIndex, elev, dmnd, dmndpat); }), "setJuncData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double elev, double initlvl, double minlvl, double maxlvl, double diam, double minvol, cweeStr volcurve) { proj->setTankData(nodeIndex, elev, initlvl, minlvl, maxlvl, diam, minvol, volcurve); }), "setTankData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double x, double y) { proj->setCoord(nodeIndex, x, y); }), "setCoord");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getNodeindex(id); }), "getNodeindex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNodeid(nodeIndex); }), "getNodeid");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNodetype(nodeIndex); }), "getNodetype");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int property) { return proj->getNodevalue(nodeIndex, property); }), "getNodevalue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getCoordX(nodeIndex); }), "getCoordX");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getCoordY(nodeIndex); }), "getCoordY");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double baseDemand, cweeStr demandPattern, cweeStr demandName) { proj->addDemand(nodeIndex, baseDemand, demandPattern, demandName); }), "addDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { proj->deleteDemand(nodeIndex, demandIndex); }), "deleteDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex, double baseDemand) { proj->setBaseDemand(nodeIndex, demandIndex, baseDemand); }), "setBaseDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex, int patIndex) { proj->setDemandPattern(nodeIndex, demandIndex, patIndex); }), "setDemandPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIdx, cweeStr demandName) { proj->setDemandName(nodeIndex, demandIdx, demandName); }), "setDemandName");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, cweeStr demandName) { return proj->getDemandIndex(nodeIndex, demandName); }), "getDemandIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNumDemands(nodeIndex); }), "getNumDemands");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getBaseDemand(nodeIndex, demandIndex); }), "getBaseDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getDemandPattern(nodeIndex, demandIndex); }), "getDemandPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getDemandName(nodeIndex, demandIndex); }), "getDemandName");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->ParseNetwork(); }), "ParseNetwork");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->RemoveDeadEnds(); }), "RemoveDeadEnds");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->CombineBasicPipesInSeries(); }), "CombineBasicPipesInSeries");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, ::epanet::Pzone const& z) { proj->CollapseZone(z); }), "CollapseZone");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, ::epanet::Pzone const& z, cweeStr mode, double val) { proj->DemandRedistribution(z, mode, val); }), "DemandRedistribution");

                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, int linkType, cweeStr fromNode, cweeStr toNode) { return proj->addLink(id, linkType, fromNode, toNode); }), "addLink");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int actionCode) { proj->deleteLink(index, actionCode); }), "deleteLink");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int property, double value) { proj->setLinkValue(index, property, value); }), "setLinkValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double length, double diam, double rough, double mloss) { proj->setPipeData(index, length, diam, rough, mloss); }), "setPipeData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr newid) { proj->setLinkID(index, newid); }), "setLinkID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int* inout_index, int linkType, int actionCode) { proj->setLinkType(inout_index, linkType, actionCode); }), "setLinkType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int node1, int node2) { proj->setLinkNodes(index, node1, node2); }), "setLinkNodes");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkUpstreamNode(index); }), "getlinkUpstreamNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkDownstreamNode(index); }), "getlinkDownstreamNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int property) { return proj->getLinkValue(index, property); }), "getLinkValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getlinkIndex(id); }), "getlinkIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkID(index); }), "getlinkID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getLinkType(index); }), "getLinkType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex) { return proj->getPumpType(linkIndex); }), "getPumpType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex) { return proj->getHeadCurveIndex(linkIndex); }), "getHeadCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex, int curveIndex) { proj->setHeadCurveIndex(linkIndex, curveIndex); }), "setHeadCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { proj->addPattern(id); }), "addPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { proj->deletePattern(index); }), "deletePattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr id) { proj->setPatternID(index, id); }), "setPatternID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period, double value) { proj->setPatternValue(index, period, value); }), "setPatternValue");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double* values, int len) { proj->setPattern(index, values, len); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getPatternIndex(id); }), "getPatternIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPatternID(index); }), "getPatternID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPatternLen(index); }), "getPatternLen");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period) { return proj->getPatternValue(index, period); }), "getPatternValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getAveragePatternValue(index); }), "getAveragePatternValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumPatterns(); }), "getNumPatterns");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPattern(index); }), "getPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeThreadedList<float> data) { proj->setPattern(index, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeThreadedList<std::pair<u64, float>> data) { proj->setPattern(index, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, cweeThreadedList<float> data) { proj->setPattern(id, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, cweeThreadedList<std::pair<u64, float>> data) { proj->setPattern(id, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { proj->addCurve(id); }), "addCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { proj->deleteCurve(index); }), "deleteCurve");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double* xValues, double* yValues, int nPoints) { proj->setCurve(index, xValues, yValues, nPoints); }), "setCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr id) { proj->setCurveID(index, id); }), "setCurveID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int curveIndex, int pointIndex, double x, double y) { proj->setCurveValue(curveIndex, pointIndex, x, y); }), "setCurveValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getCurveIndex(id); }), "getCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveID(index); }), "getCurveID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period) { return proj->getCurveValue(index, period); }), "getCurveValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveLen(index); }), "getCurveLen");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveType(index); }), "getCurveType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumCurves(); }), "getNumCurves");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurve(index); }), "getCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumControls(); }), "getNumControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlIndex) { proj->deleteControl(controlIndex); }), "deleteControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->deleteAllControls(); }), "deleteAllControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlType, cweeStr linkName, float setting, cweeStr nodeName, float level) { return proj->addControl(controlType, linkName, setting, nodeName, level); }), "addControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr entireControl) { return proj->addControl(entireControl); }), "addControl");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlindex, int* type, int* linkIndex, double* setting, int* nodeIndex, double* level) { proj->getControl(controlindex, type, linkIndex, setting, nodeIndex, level); }), "getControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlindex) { return proj->getControl(controlindex); }), "getControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumRules(); }), "getNumRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr ruleName) { return proj->findRuleIndex(ruleName); }), "findRuleIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getRuleID(ruleIndex); }), "getRuleID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, cweeStr linkIndex, cweeStr linkValue, cweeStr ifTrueThenStatusOrSetting, cweeStr refObjectType, cweeStr refObjectIndex, cweeStr refObjectVariable, cweeStr ComparisonOperator, cweeStr refObjectStatusOrValue, double priority) { return proj->addRule(name, linkIndex, linkValue, ifTrueThenStatusOrSetting, refObjectType, refObjectIndex, refObjectVariable, ComparisonOperator, refObjectStatusOrValue, priority); }), "addRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr ruleName, cweeStr entireRule, bool returnIndex = true) { return proj->addRule(ruleName, entireRule, returnIndex = true); }), "addRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { proj->deleteRule(ruleIndex); }), "deleteRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->deleteAllRules(); }), "deleteAllRules");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int* nPremises = nullptr, int* nThenActions = nullptr, int* nElseActions = nullptr, double* priority = nullptr) { proj->getRuleSizes(ruleIndex, nPremises = nullptr, nThenActions = nullptr, nElseActions = nullptr, priority = nullptr); }), "getRuleSizes");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int premiseIndex, int premiseType, int objectType, int objectIndex, int objectVariable, int comparisonOp, int comparisonStatus, float comparisonValue) { proj->setRule_Premise(ruleIndex, premiseIndex, premiseType, objectType, objectIndex, objectVariable, comparisonOp, comparisonStatus, comparisonValue); }), "setRule_Premise");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int actionIndex, int linkIndex, int status, double setting) { proj->setRule_ElseAction(ruleIndex, actionIndex, linkIndex, status, setting); }), "setRule_ElseAction");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double priority) { proj->setRule_Priority(index, priority); }), "setRule_Priority");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int actionIndex, int linkIndex, int status, double setting) { proj->setRule_ThenAction(ruleIndex, actionIndex, linkIndex, status, setting); }), "setRule_ThenAction");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getRule(ruleIndex); }), "getRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getRules(); }), "getRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int projectNum, cweeStr name, bool isNode) { return proj->getEPAnetIndexOfAsset(projectNum, name, isNode); }), "getEPAnetIndexOfAsset");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, asset_t type) { return proj->isNode(type); }), "isNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int EPAnetIndex, bool isNode, int prop) { return proj->getCurrentHydraulicValue(EPAnetIndex, isNode, prop); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int EPAnetIndex, value_t value) { return proj->getCurrentHydraulicValue(EPAnetIndex, value); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, value_t value, bool isNode) { return proj->getCurrentHydraulicValue(name, value, isNode); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, value_t value) { return proj->getCurrentHydraulicValue(name, value); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getImpactedAssetsFromRule(ruleIndex); }), "getImpactedAssetsFromRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getImpactedAssetsFromRules(); }), "getImpactedAssetsFromRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getImpactedAssetsFromControls(); }), "getImpactedAssetsFromControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, Pattern* pat, cweeStr name, u64 start, u64 end, u64 stepSeconds) { return proj->InsertPattern(pat, name, start, end, stepSeconds); }), "InsertPattern");















                    }
                    {
                        lib->add(chaiscript::user_type<cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>>(), "EPAnetHydraulicSim");
                        lib->add(constructor<cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>(const cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>&)>(), "EPAnetHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>& a, const cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>& b) { a = b; return a; }), "=");

                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& a) -> bool { 
                            auto hyd = a->StartHydraulicSimulation(); 
                            while (true) {
                                hyd->DoSteadyState();
                                if (a->getCurrentError() > 100) { return false; }
                                if (!hyd->ShouldContinueSimulation()) { break; }
                            }
                            return true;
                        }), "DoHydraulicSimulation");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& a) { return a->StartHydraulicSimulation(); }), "StartHydraulicSimulation");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a, cweeUnitValues::minute minutes) { a->SetTimestep(minutes()); }), "SetTimestep");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { return a->GetCurrentSimTime(); }), "GetCurrentSimTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { a->DoSteadyState(); }), "DoSteadyState");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { return a->ShouldContinueSimulation(); }), "ShouldContinueSimulation");
                    }
                }

                /* EPAnet optimization */ {
                    AUTO lambda = [](cweeSharedPtr<EPAnetProject>& control_project, int numValves, cweeStr method, int numPolicies) {
                        ParticleSwarm_OptimizationManagementTool<true> // Alternating_OptimizationManagementTool< Random_OptimizationManagementTool<true>, Genetic_OptimizationManagementTool<true>>
                            ramtT(numValves, numPolicies);

                        const auto PenaltyValue = ramtT.default_constraint();

                        cweeSharedPtr<cweeThreadedMap<std::string, double>>  cachedResults = make_cwee_shared<cweeThreadedMap<std::string, double>>();

                        units::flowrate::gallon_per_minute_t minFlowrate = 40;

                        // skeletonize the control project                         
                        // control_project->RemoveDeadEnds();

                        // simulate the control project
                        {
                            AUTO sim = control_project->StartHydraulicSimulation();
                            while (true) {
                                sim->DoSteadyState();
                                if (!sim->ShouldContinueSimulation()) break;
                            }
                        }
                        AUTO control_network = control_project->epanetProj->network;

                        auto fp = fileSystem->createRandomFile(fileType_t::INP);
                        control_project->saveINP(fp); // save to a temp file

                        units::energy::megawatt_hour_t control_tot;
                        units::energy::megawatt_hour_t control_tot_pump; {
                            for (auto& x : control_network->Pump) { if (x) { control_tot += x->Energy.KwHrs; control_tot_pump += x->Energy.KwHrs; } }
                            for (auto& x : control_network->Valve) { if (x) { control_tot += x->Energy.KwHrs; } }
                        }

                        // units::power::kilowatt_t minPowerProduction = 10;
                        units::pressure::pounds_per_square_inch_t minAllowedCustomerPSI = units::pressure::pounds_per_square_inch_t(40.0);
                        units::pressure::pounds_per_square_inch_t minAllowedPSI = units::pressure::pounds_per_square_inch_t(5.0);
                        units::pressure::pounds_per_square_inch_t minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                        units::pressure::pounds_per_square_inch_t avgCustomerPSI = 0;
                        int pressureCount = 0;

                        for (auto& zone : control_network->Zone) {
                            if (zone && zone->HasWaterDemand()) {
                                for (auto& node : zone->Node) {
                                    if (node && node->HasWaterDemand()) {
                                        AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                        cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(avgCustomerPSI, thisPSI, pressureCount);
                                        minCustomerPSI = std::min(minCustomerPSI, thisPSI);
                                        // minAllowedCustomerPSI = std::min(minAllowedCustomerPSI, thisPSI);
                                    }
                                }
                            }
                        }

                        int nLinks = control_network->Nlinks;

                        cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_index = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                        cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_x = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                        cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_y = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                        for (int i = 1; i < control_network->Link.size(); i++) {
                            AUTO link = control_network->Link[i];
                            if (link) {
                                coord_to_index->AddValue(link->X()(), link->Y()(), i);
                                coord_to_x->AddValue(link->X()(), link->Y()(), link->X()());
                                coord_to_y->AddValue(link->X()(), link->Y()(), link->Y()());
                            }
                        }

                        constexpr int maxIterations = 1000; // 1000 iterations
                        constexpr float eps = 0.000001f;

                        using sharedObjType = ChaiscriptOptimizationObj; // cweeOptimizer::sharedClass;

                        cweeThreadedMap < std::string, units::pressure::pounds_per_square_inch_t > controlNodeToMinCustomerZonePressure;
                        /* average customer pressure */ units::pressure::pounds_per_square_inch_t controlCustomerPressure = 0; { int count = 0;
                        for (auto& zone : control_network->Zone) {
                            units::pressure::pounds_per_square_inch_t controlZonePressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t >::max();
                            units::pressure::pounds_per_square_inch_t controlZoneCustomerPressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t >::max();
                            if (zone) {
                                for (auto& node : zone->Node) {
                                    if (node) {
                                        AUTO pat = node->GetValue<_HEAD_>();
                                        if (pat) {
                                            units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                            units::pressure::head_t head = head_ft();
                                            units::pressure::pounds_per_square_inch_t thisPressure = head;

                                            if (node->HasWaterDemand()) {
                                                cweeMath::rollingAverageRef(controlCustomerPressure, thisPressure, count);
                                                controlZoneCustomerPressure = std::min(controlZoneCustomerPressure, thisPressure);
                                            }
                                            controlZonePressure = std::min(controlZonePressure, thisPressure);
                                        }
                                    }
                                }
                                for (auto& node : zone->Node) {
                                    if (node) {
                                        if (node->HasWaterDemand()) {
                                            controlNodeToMinCustomerZonePressure.Emplace(node->Name_p.c_str(), controlZoneCustomerPressure);
                                        }
                                        else {
                                            controlNodeToMinCustomerZonePressure.Emplace(node->Name_p.c_str(), controlZonePressure);
                                        }
                                    }
                                }
                            }
                        }
                        }

                        AUTO linkIsGoodCandidate = [=](::epanet::Plink link) -> bool {
                            if (link) {
                                if (link->Type_p == asset_t::PUMP) { // cannot replace a pump
                                    return false;
                                }
                                if (link->Type >= ::epanet::LinkType::PRV) {
                                    // existing prv's and fcv's are decent options considering they already reduce pressure - question is are they open? 
                                    if (link->Kc == ::epanet::MISSING) {
                                        if ((::epanet::StatusType)(double)link->Status(control_project->epanetProj->times.GetSimStartTime()) == ::epanet::OPEN)
                                        {
                                            return true; // OPEN VALVE
                                        }
                                        if ((::epanet::StatusType)(double)link->Status(control_project->epanetProj->times.GetSimStartTime()) == ::epanet::CLOSED)
                                        {
                                            return false; // CLOSED VALVE
                                        }
                                    }
                                    return false; // UNKNOWN
                                }

                                AUTO flowPat = link->GetValue<_FLOW_>();
                                // AUTO min_flow = flowPat->GetMinValue();
                                AUTO max_flow = flowPat->GetAvgValue(); // was GetMaxValue
                                if (units::math::fabs(max_flow) < minFlowrate) {
                                    return false; // not enough flow to warrant analysis
                                }

                                return true;
                            }
                            return false;
                        };
                        AUTO getNextGoodLink = [=](::epanet::Plink link) -> ::epanet::Plink {
                            if (link && linkIsGoodCandidate(link)) {
                                if (link->GetValue<_FLOW_>()->GetAvgValue() > units::flowrate::gallon_per_minute_t(0)) { // positive direction
                                    AUTO links = control_project->getConnectedLinks(link->N1); // upstream
                                    links.RemoveFast(link);
                                    if (links.size() == 1 && links[0]) {
                                        if (linkIsGoodCandidate(links[0])) {
                                            // this connecting pipe appears to be a good candidate. Make sure we don't cause a loop
                                            // by finding pipes that connect to a node that happen to both be feeding it. I.e.:
                                            // -->--> * <--<--
                                            // We want:
                                            // -->--> * -->-->

                                            if (links[0]->GetDownstreamNode()->Name_p == link->StartingNode->Name_p) {
                                                // good! 
                                                return links[0];

                                            }
                                            else {
                                                // bad!
                                                return link;
                                            }
                                        }
                                        else {
                                            return link;
                                        }
                                    }
                                    else {
                                        // this pipe connects to a tree -> unclear how to handle, so accept this pipe
                                        return link;
                                    }
                                }
                                else { // negative direction
                                    AUTO links = control_project->getConnectedLinks(link->N2); // downstream
                                    links.RemoveFast(link);
                                    if (links.size() == 1 && links[0]) {
                                        if (linkIsGoodCandidate(links[0])) {
                                            // this connecting pipe appears to be a good candidate. Make sure we don't cause a loop
                                            // by finding pipes that connect to a node that happen to both be feeding it. I.e.:
                                            // -->--> * <--<--
                                            // We want:
                                            // -->--> * -->-->

                                            if (links[0]->GetDownstreamNode()->Name_p == link->EndingNode->Name_p) {
                                                // good!
                                                return link;
                                            }
                                            else {
                                                // bad!
                                                return links[0];
                                            }
                                        }
                                        else {
                                            return link;
                                        }
                                    }
                                    else {
                                        // this pipe connects to a tree -> unclear how to handle, so accept this pipe
                                        return link;
                                    }

                                }
                            }
                            return link;
                        };

                        AUTO createEPAnetProject = [=](cweeThreadedList<float> const& policy, cweeList<::epanet::Plink>& newLinks, cweeStr& policyStr)->cweeSharedPtr<EPAnetProject> {
                            AUTO p = EPAnet->createNewProject();
                            p->loadINP(fp);

                            policyStr.Clear();
                            newLinks.Clear();
                            cweeList<int> indexes;
                            cweeThreadedMap<std::string, units::flowrate::gallon_per_minute_t> averageFlowrateMap;
                            for (auto& hilbertPos : policy) {
                                int link_index = cweeMath::Rint(coord_to_index->HilbertPositionToValue(hilbertPos));

                                auto newLinkName = "NewLink" + cweeStr(cweeRandomInt(0, 10000));
                                try {
                                    auto control_link2 = control_network->Link[link_index];
                                    if (control_link2) {
                                        auto control_link = control_link2;
#if 0
                                        int preventLoop = 25;
                                        // move upstream/downstream to get the best link that starts this path
                                        while (control_link2 && control_link) {
                                            control_link2 = getNextGoodLink(control_link);
                                            if (control_link == control_link2) {
                                                break;
                                            }
                                            else {
                                                // only accept the recommendation if our elevation DID NOT increase. 
                                                if (
                                                    (control_link->StartingNode->El + control_link->EndingNode->El)
                                                    >=
                                                    (control_link2->StartingNode->El + control_link2->EndingNode->El)
                                                    ) {
                                                    control_link = control_link2;
                                                }
                                                else {
                                                    break;
                                                }
                                            }
                                            if (--preventLoop <= 0) break;
                                        }
#endif
                                        if (control_link) {
                                            // the control_link now determines the link_index;
                                            link_index = control_project->getlinkIndex(control_link->Name_p);

                                            // policyStr.AddToDelimiter(link_index, ", ");

                                            if (indexes.Find(link_index)) {
                                                // cannot repeat indexes! Makes no sense.
                                                continue; // return nullptr;
                                            }
                                            else {
                                                indexes.Append(link_index);
                                            }

                                            AUTO flowPat = control_link->GetValue<_FLOW_>();
                                            if (flowPat) {
                                                units::flowrate::gallon_per_minute_t avgFlow = flowPat->GetAvgValue();
                                                if (avgFlow > units::flowrate::gallon_per_minute_t(0)) { // positive direction
                                                    AUTO downstreamPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((control_link->EndingNode->GetValue<_HEAD_>()->GetAvgValue() - control_link->EndingNode->El)());
                                                    auto linkID = p->addLink(
                                                        newLinkName,
                                                        3, // 3 = PRV, 7 = TCV
                                                        control_link->StartingNode->Name_p,
                                                        control_link->EndingNode->Name_p
                                                    ); // name, type, fromName, toName                                                
                                                    if (linkID > 0) {
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                            1.0 // true
                                                        );
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                            downstreamPSI()
                                                        );
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_DIAMETER,
                                                            ((units::length::inch_t)(control_link->Diam))()
                                                        );
                                                        // accept the vertices as-is. 
                                                        AUTO destLink = p->epanetProj->network->Link[linkID];
                                                        if (destLink) {
                                                            destLink->Vertices = p->epanetProj->network->Link[link_index]->Vertices; // the vertices are good as-is
                                                            newLinks.Append(destLink);
                                                        }
                                                    }
                                                    else {
                                                        // there are places that are bad for valves or invalid -- for example, direct connection to Reservoirs.
                                                        return nullptr;
                                                    }
                                                }
                                                else { // reverse direction
                                                    AUTO downstreamPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((control_link->StartingNode->GetValue<_HEAD_>()->GetAvgValue() - control_link->StartingNode->El)());
                                                    auto linkID = p->addLink(
                                                        newLinkName,
                                                        3, // 3 = PRV, 7 = TCV
                                                        control_link->EndingNode->Name_p,
                                                        control_link->StartingNode->Name_p
                                                    ); // name, type, fromName, toName
                                                    if (linkID > 0) {
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                            1.0 // true
                                                        );
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                            downstreamPSI()
                                                        );
                                                        p->setLinkValue(
                                                            linkID, // index
                                                            ::epanet::EN_LinkProperty::EN_DIAMETER,
                                                            ((units::length::inch_t)(control_link->Diam))()
                                                        );
                                                        AUTO thisLink = p->epanetProj->network->Link[link_index];
                                                        AUTO destLink = p->epanetProj->network->Link[linkID];
                                                        if (destLink && thisLink && thisLink->Vertices) {
                                                            // the vertices need to be reversed to make any sense
                                                            thisLink->Vertices->Array.Reverse();
                                                            // copies the list to the end, then fast-swaps the first with the last items and results 
                                                            // in a reversed list.

                                                            destLink->Vertices = thisLink->Vertices;
                                                        }
                                                        if (destLink) {
                                                            newLinks.Append(destLink);
                                                        }
                                                    }
                                                    else {
                                                        // there are places that are bad for valves or invalid -- for example, direct connection to Reservoirs.
                                                        return nullptr;
                                                    }
                                                }
                                                averageFlowrateMap.Emplace(newLinkName.c_str(), units::math::fabs(avgFlow));
                                                p->deleteLink(link_index, 0);
                                            }
                                        }
                                    }
                                }
                                catch (...) {}
                            }
                            indexes.Sort();
                            for (auto& index : indexes) {
                                policyStr.AddToDelimiter(index, ", ");
                            }

                            /* Regenerate zones after changing pipes */
                            p->ParseNetwork();

                            /* Check if the network is physically possible. */
                            for (auto& zone : p->epanetProj->network->Zone) {
                                if (zone && zone->IllDefined != ::epanet::illDefined_t::Okay) {
                                    return nullptr;
                                }
                            }

                            /* Check if one of the valves are essentially useless */
                            for (auto& link : newLinks) {
                                if (link && (link->StartingNode->Zone == link->EndingNode->Zone)) {
                                    return nullptr;
                                }
                            }

                            // optimize the setting for the new turbines or valves to reduce pressure as much as possible                                
                            for (auto& link : newLinks) {
                                if (link && link->EndingNode) {
                                    // for each of the new links, go through their downstream zone nodes
                                    AUTO downstream_zone = link->EndingNode->Zone;
                                    if (downstream_zone && downstream_zone->HasWaterDemand()) {
                                        units::pressure::pounds_per_square_inch_t minZonePressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                        units::pressure::pounds_per_square_inch_t minZoneCustomerPressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                        for (auto& node : downstream_zone->Node) {
                                            if (node) {
                                                // find this node in the control network to retrieve its pressure                                                    
                                                int index = control_project->getNodeindex(node->Name_p);
                                                if (index > 0) {
                                                    AUTO thisPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(((units::length::foot_t)(control_network->Node[index]->GetValue<_HEAD_>()->GetMinValue() - control_network->Node[index]->El))());
                                                    if (node->HasWaterDemand()) {
                                                        if (minZonePressure > thisPressure) minZonePressure = thisPressure;
                                                        if (minZoneCustomerPressure > thisPressure) minZoneCustomerPressure = thisPressure;
                                                    }
                                                    else {
                                                        if (minZonePressure > thisPressure) minZonePressure = thisPressure;
                                                    }
                                                }
                                            }
                                        }

                                        if (minZoneCustomerPressure > minAllowedCustomerPSI && minZonePressure > minAllowedPSI) {
                                            // there is room to further decrease the pressure, potentially.
                                            AUTO diff1 = minZoneCustomerPressure - minAllowedCustomerPSI; // i.e. 65psi - 40psi = 25psi
                                            AUTO diff2 = minZonePressure - minAllowedPSI; // i.e. 25psi - 5psi = 20psi
                                            units::pressure::pounds_per_square_inch_t diff = diff1 < diff2 ? diff1 : diff2; // i.e. 20psi

                                            // estimate the energy production of this valve:
                                            units::pressure::head_t H = diff;
                                            units::length::foot_t H_ft = H();
                                            AUTO f_ptr = averageFlowrateMap.TryGetPtr(link->Name_p.c_str());
                                            if (f_ptr) {
                                                AUTO kW_potential = cweeEng::CentrifugalPumpEnergyDemand_kW(*f_ptr, H_ft, 131);
                                                if (kW_potential < (units::power::kilowatt_t)(50)) {
                                                    // this valve likely does not produce enough energy to be meaningful or a worthwhile ERT. BUT it could still be a PRV!
                                                    p->setLinkValue(
                                                        p->getlinkIndex(link->Name_p), // index
                                                        ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                        0.0
                                                    );
                                                }
                                            }

                                            if (diff > (units::pressure::pounds_per_square_inch_t)0) {
                                                p->setLinkValue( // change the previous setting (whatever it was) by reducing it as much as we could
                                                    p->getlinkIndex(link->Name_p), // index
                                                    ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                    p->getLinkValue(p->getlinkIndex(link->Name_p), ::epanet::EN_LinkProperty::EN_INITSETTING) - diff()
                                                );
                                            }
                                        }
                                        else {
                                            // there is no room to further reduce pressure -- this position was never going to work.

                                        }
                                    }

                                }
                            }
                            return p;
                        };

                        AUTO evaluate = [=](cweeSharedPtr<EPAnetProject> proj, cweeList<::epanet::Plink> newLinks) -> cweeThreadedMap<std::string, cweeUnitValues::unit_value> {
                            using namespace cwee_units;
                            scalar_t penalty = 0;

                            /* simulation */ {
                                // proj->epanetProj->times.Dur /= 7.0;
                                AUTO sim = proj->StartHydraulicSimulation();
                                while (true) {
                                    sim->DoSteadyState(); // ::epanet::HydraulicSimulationQuality::LOWRES);
                                    if (proj->getCurrentError() >= 100) {
                                        // bad simulation -- big penalty.
                                        penalty = PenaltyValue;
                                        break;
                                    }
                                    if (!sim->ShouldContinueSimulation()) break;
                                }
                                // proj->epanetProj->times.Dur *= 7.0;
                            }

                            // if more cost effective, fix the PRV/ERT distinction.
#if 0
                            Dollar_t NPV_valves = 0_USD;
                            for (auto& link : newLinks) {
                                if (link && link->Type_p == asset_t::VALVE) {
                                    AUTO valveP = link.CastReference<::epanet::Svalve>();
                                    if (valveP) {
                                        AUTO NPV_PRV = proj->epanetProj->network->Leakage.NetPresentValueOfValve(valveP, 1);
                                        AUTO NPV_PAT = proj->epanetProj->network->Leakage.NetPresentValueOfValve(valveP, 2);

                                        if (NPV_PRV > NPV_PAT) {
                                            valveP->ProducesElectricity = false;
                                            link->GetValue<_ENERGY_>()->operator=(0_kW); // zero-out the energy generation
                                        }
                                        else {
                                            valveP->ProducesElectricity = true;
                                        }

                                        NPV_valves += proj->epanetProj->network->Leakage.NetPresentValueOfValve(valveP);
                                    }
                                }
                            }
#endif

                            // pressure penalties
                            for (auto& zone : proj->epanetProj->network->Zone) {
                                if (zone && zone->HasWaterDemand()) {
                                    for (auto& node : zone->Node) {
                                        if (node) {
                                            AUTO pat = node->GetValue<_HEAD_>();
                                            if (pat) {
                                                units::pressure::pounds_per_square_inch_t thisPressure;
                                                {
                                                    units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                                    units::pressure::head_t head = head_ft();
                                                    thisPressure = head;
                                                }

                                                AUTO minPressure = controlNodeToMinCustomerZonePressure.TryGetPtr(node->Name_p.c_str());
                                                if (minPressure && thisPressure < minAllowedCustomerPSI) {
                                                    // We reduced pressure more than the original pressure (Possibly OK). Is it unacceptable though? 
                                                    if (thisPressure < minAllowedPSI) {
                                                        // no node should be below 5 psi
                                                        penalty += (minAllowedPSI - thisPressure)();
                                                    }
                                                    if (node->HasWaterDemand() && thisPressure < *minPressure) {
                                                        // no customer should recieve less than 40psi
                                                        penalty += (*minPressure - thisPressure)(); // minAllowedCustomerPSI - thisPressure
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // total energy used minus total energy generated
                            kilowatt_hour_t totalEnergyDemand = 0; {
                                for (auto& link : proj->epanetProj->network->Link) {
                                    if (link) {
                                        AUTO pat = link->GetValue<_ENERGY_>();
                                        if (pat) {
                                            totalEnergyDemand += pat->RombergIntegral(pat->GetMinTime(), pat->GetMaxTime());
                                        }
                                    }
                                }
                            }

                            // average water pressure
                            pounds_per_square_inch_t customerPressure = 0; { int count = 0;
                            for (auto& zone : proj->epanetProj->network->Zone) {
                                if (zone && zone->HasWaterDemand()) {
                                    for (auto& node : zone->Node) {
                                        if (node && node->HasWaterDemand()) {
                                            AUTO pat = node->GetValue<_HEAD_>();
                                            if (pat) {
                                                units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                                units::pressure::head_t head = head_ft();
                                                units::pressure::pounds_per_square_inch_t thisPressure = head;

                                                cweeMath::rollingAverageRef(customerPressure, thisPressure, count);
                                            }
                                        }
                                    }
                                }
                            }
                            }

                            // reduction to average customer pressure
                            pounds_per_square_inch_t customerPressureDelta = controlCustomerPressure - customerPressure;

#if 0
                            Dollar_t NPV_zones = 0_USD;
                            for (auto& zone : proj->epanetProj->network->Zone) {
                                ::epanet::Pzone control_zone = nullptr; {
                                    // find this zone in the control network if possible. If not possible, find one with shared links. If none of the above work, we'll simply not "reduce" pressure in the new zone.
                                    if (!control_zone) {
                                        for (auto& czone : control_network->Zone) {
                                            // best case scenario
                                            if (czone->Name_p == zone->Name_p) {
                                                control_zone = czone;
                                                break;
                                            }
                                        }
                                    }
                                    if (!control_zone) {
                                        for (auto& node : zone->Node) {
                                            for (auto& cnode : control_network->Node) {
                                                // best case scenario
                                                if (cnode->Name_p == node->Name_p) {
                                                    control_zone = cnode->Zone;
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }

                                Dollar_t NPV_zone = 0_USD; {
                                    if (control_zone) {
                                        AUTO cP = control_zone->AverageNodePressure();
                                        year_t surveyFreq = zone->SurveyFrequency(proj->epanetProj, cP);
                                        NPV_zone = zone->LeakModelResults(proj->epanetProj, surveyFreq, cP)["Net Benefits"]();
                                    }
                                    else {
                                        AUTO cP = zone->AverageNodePressure();
                                        year_t surveyFreq = zone->SurveyFrequency(proj->epanetProj, cP);
                                        NPV_zone = zone->LeakModelResults(proj->epanetProj, surveyFreq, cP)["Net Benefits"]();
                                    }
                                }
                                NPV_zones += NPV_zone;
                            }
#endif

                            // Final Performance Results
                            Dollar_t performance = 0; {
                                //performance += NPV_valves;
                                //performance += NPV_zones;

                                if (std::isnan((performance + (Dollar_t)((std::min(std::max(0.0, penalty()), (double)numValves) / (double)numValves) * PenaltyValue))())) {
                                    performance = PenaltyValue;
                                }
                                else {
                                    performance += (Dollar_t)((std::min(std::max(0.0, penalty()), (double)numValves) / (double)numValves) * PenaltyValue);
                                }
                            }

                            cweeThreadedMap<std::string, cweeUnitValues::unit_value> results;
                            results.Emplace("Penalty", penalty);
                            results.Emplace("Energy Demand", totalEnergyDemand);
                            results.Emplace("Customer Pressure", customerPressure);
                            results.Emplace("Pressure Delta", customerPressureDelta);
                            results.Emplace("Performance", performance);
                            //results.Emplace("Net Benefits (Zones)", NPV_zones);
                            //results.Emplace("Net Benefits (Valves)", NPV_valves);
                            //results.Emplace("Net Benefits", NPV_zones + NPV_valves);

                            return results;
                        };

                        cweeSharedPtr<cweeThreadedList<float>> BestPolicy = make_cwee_shared<cweeThreadedList<float>>();
                        cweeSharedPtr<float> bestPerformance = std::numeric_limits<float>::max();

                        /* policies are positions along the hilbert curve. */
                        AUTO todo = [=](cweeThreadedList<float> const& policy)-> float {
                            double performance = 0.0;
                            performance = PenaltyValue;

                            cweeStr policyStr; {
                                cweeList<int> sortedIndexes1;
                                for (auto& hilbertPos : policy) {
                                    int link_index = cweeMath::Rint(coord_to_index->HilbertPositionToValue(hilbertPos));
                                    sortedIndexes1.Append(link_index);
                                }
                                sortedIndexes1.Sort();
                                for (auto& link_index : sortedIndexes1) {
                                    policyStr.AddToDelimiter(link_index, ", ");
                                }
                            }

                            /* Check if we've seen this suggested placement before */
                            AUTO cachedPerformance = cachedResults->TryGetPtr(policyStr.c_str());
                            if (cachedPerformance) {
                                performance = *cachedPerformance;
                                return performance;
                            }

                            /* Generate the new EPAnet model. NOTE that it may change the placement of the valves as an improvement to the suggestion. */
                            cweeList<::epanet::Plink> newLinks; cweeStr newPolicyStr;
                            AUTO p = createEPAnetProject(policy, newLinks, newPolicyStr);
                            if (!p || newLinks.size() != numValves) { // newPolicyStr is unusable in this state                                    
                                performance = PenaltyValue;
                                cachedResults->Emplace(policyStr.c_str(), performance);
                                return performance;
                            }

                            /* Check if we've seen this revised valve placement before. */
                            AUTO cachedPerformance2 = cachedResults->TryGetPtr(newPolicyStr.c_str());
                            if (cachedPerformance2) {
                                performance = *cachedPerformance2;
                                cachedResults->Emplace(policyStr.c_str(), performance);
                                return performance;
                            }

                            /* The placement appears to be valid. Let's try it */
                            AUTO sim_result = evaluate(p, newLinks);

                            performance = sim_result.TryGetPtr("Performance")->operator()();

                            units::pressure::pounds_per_square_inch_t this_minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                            units::pressure::pounds_per_square_inch_t this_avgCustomerPSI = 0;
                            /* Evaluate water customer pressures */ {
                                int this_pressureCount = 0;
                                for (auto& zone : p->epanetProj->network->Zone) {
                                    if (zone && zone->HasWaterDemand()) {
                                        for (auto& node : zone->Node) {
                                            if (node && node->HasWaterDemand()) {
                                                AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                                cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(this_avgCustomerPSI, thisPSI, this_pressureCount);
                                                this_minCustomerPSI = std::min(this_minCustomerPSI, thisPSI);
                                            }
                                        }
                                    }
                                }

                                if (this_minCustomerPSI < units::pressure::pounds_per_square_inch_t(0)) {
                                    // very bad customer pressure
                                    performance += (((units::pressure::pounds_per_square_inch_t(0) - this_minCustomerPSI) / units::pressure::pounds_per_square_inch_t(1)) * units::energy::megawatt_hour_t(5000))(); // 50MWh penalty per min pressure reduction
                                }
                                if (this_minCustomerPSI < minCustomerPSI) {
                                    // bad customer pressure
                                    performance += (((minCustomerPSI - this_minCustomerPSI) / units::pressure::pounds_per_square_inch_t(1)) * units::energy::megawatt_hour_t(500))(); // 50MWh penalty per min pressure reduction
                                }
                            }

                            cachedResults->Emplace(policyStr.c_str(), performance);
                            cachedResults->Emplace(newPolicyStr.c_str(), performance);
                            return performance;
                        };
                        AUTO objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
                            return todo(policy);
                        };
                        AUTO isFinishedFunc = [=](sharedObjType& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformanceV) -> bool {
                            auto& i = shared.results.Alloc();
                            i.bestPolicy = bestPolicy;
                            i.bestPerformance = bestPerformanceV;

                            bestPerformance.Lock();
                            if (*bestPerformance.UnsafeGet() > bestPerformanceV) {
                                *bestPerformance.UnsafeGet() = bestPerformanceV;

                                BestPolicy.Lock();
                                *BestPolicy.UnsafeGet() = bestPolicy;
                                BestPolicy.Unlock();
                            }
                            bestPerformance.Unlock();

                            if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
                            else {
                                // ensure the most recent 5 iterations are better than the 5 before them.
                                int i = shared.results.Num() - 1;
                                float perf_new = 0;
                                float perf_old = 0;
                                int count = 0;
                                for (; i >= (shared.results.Num() - 5); i--) {
                                    cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
                                }
                                count = 0;
                                for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
                                    cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
                                }

                                // test for convergence
                                auto is_convergent = [=](double sum, double prev_sum)->bool {
                                    double epsilon = eps; // Tolerance
                                    return std::abs(sum - prev_sum) < epsilon;
                                };

                                if (is_convergent(perf_new, perf_old)) {
                                    shared.numIterationsFailedImprovement++;
                                    if (shared.numIterationsFailedImprovement > 20) // (maxIterations / 10)) // 20 iterations processed and we saw no further improvement. Unlikely to see improvement with another 20.                         
                                        return true;
                                    else
                                        return false;
                                }
                                shared.numIterationsFailedImprovement = 0;
                                return false; // we have not flat-lined. continue.
                            }
                        };

                        cweeJob toAwait;
                        cweeSharedPtr<sharedObjType> shared = make_cwee_shared<sharedObjType>();

                        method = method.BestMatch({ "PSO", "Genetic", "Random" });
                        switch (method.Hash()) {
                        default:
                        case cweeStr::Hash("PSO"): {
                            ParticleSwarm_OptimizationManagementTool<true> ramt(numValves, numPolicies);
                            for (int i = 0; i < numValves; i++) ramt.lower_constraints().Emplace((float)coord_to_index->MinHilbertPosition(), i);
                            for (int i = 0; i < numValves; i++) ramt.upper_constraints().Emplace((float)coord_to_index->MaxHilbertPosition(), i);
                            toAwait = cweeOptimizer::run_optimization(shared, ramt, std::function(objFunc), std::function(isFinishedFunc), maxIterations);
                            toAwait.AwaitAll();
                            break; }
                        case cweeStr::Hash("Genetic"): {
                            Genetic_OptimizationManagementTool<true> ramt(numValves, numPolicies);
                            for (int i = 0; i < numValves; i++) ramt.lower_constraints().Emplace((float)coord_to_index->MinHilbertPosition(), i);
                            for (int i = 0; i < numValves; i++) ramt.upper_constraints().Emplace((float)coord_to_index->MaxHilbertPosition(), i);
                            toAwait = cweeOptimizer::run_optimization(shared, ramt, std::function(objFunc), std::function(isFinishedFunc), maxIterations);
                            toAwait.AwaitAll();
                            break; }
                        case cweeStr::Hash("Random"): {
                            Random_OptimizationManagementTool<true> ramt(numValves, numPolicies);
                            for (int i = 0; i < numValves; i++) ramt.lower_constraints().Emplace((float)coord_to_index->MinHilbertPosition(), i);
                            for (int i = 0; i < numValves; i++) ramt.upper_constraints().Emplace((float)coord_to_index->MaxHilbertPosition(), i);
                            toAwait = cweeOptimizer::run_optimization(shared, ramt, std::function(objFunc), std::function(isFinishedFunc), maxIterations);
                            toAwait.AwaitAll();
                            break; }
                        }

                        std::map<std::string, chaiscript::Boxed_Value> bv_final;
                        try {
                            auto& bestPolicy = *BestPolicy;
                            auto& bestPerf = *bestPerformance;
                            bv_final["performance"] = chaiscript::var((float)*bestPerformance);
                            bv_final["iterations"] = chaiscript::var((int)(shared->results.Num()));

                            cweeStr policy_Str;
                            cweeList<::epanet::Plink> newLinks;
                            AUTO p = createEPAnetProject(bestPolicy, newLinks, policy_Str);
                            if (p) {
                                /* Perform the sim */ { AUTO hyd = p->StartHydraulicSimulation();
                                while (true) {
                                    hyd->DoSteadyState();
                                    if (!hyd->ShouldContinueSimulation()) {
                                        break;
                                    }
                                } }

                                auto evaluated = evaluate(p, newLinks);
                                for (auto& r : evaluated) {
                                    bv_final[r.first] = chaiscript::var(cweeUnitValues::unit_value(*r.second));
                                }

                                units::pressure::pounds_per_square_inch_t this_minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                units::pressure::pounds_per_square_inch_t this_avgCustomerPSI = 0;
                                /* Evaluate water customer pressures */ {
                                    int this_pressureCount = 0;
                                    for (auto& zone : p->epanetProj->network->Zone) {
                                        if (zone && zone->HasWaterDemand()) {
                                            for (auto& node : zone->Node) {
                                                if (node && node->HasWaterDemand()) {
                                                    AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                                    cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(this_avgCustomerPSI, thisPSI, this_pressureCount);
                                                    this_minCustomerPSI = std::min(this_minCustomerPSI, thisPSI);
                                                }
                                            }
                                        }
                                    }
                                }

                                bv_final["Avg Pressure"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(this_avgCustomerPSI)));
                                bv_final["Min Pressure"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(this_minCustomerPSI)));
                                bv_final["Avg Pressure Reduction"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(avgCustomerPSI - this_avgCustomerPSI)));
                                bv_final["Project"] = chaiscript::var(cweeSharedPtr<EPAnetProject>(p));
                            }

#if 0
                            /* display the points we reviewed into a UI_MapLayer */ {
                                UI_MapLayer layer;
                                int policyN = 0;
                                int Npolicies = shared->policies.size();
                                int numPoliciesPerIteration = 12;
                                UI_Color c;
                                for (cweeList<float>& policy : shared->policies) {
                                    policyN++;
                                    double b = (double)policyN / (double)Npolicies;
                                    if (policyN % numPoliciesPerIteration == 0) {
                                        c = UI_Color(
                                            cweeMath::max(0, 255.0 * (b)),
                                            cweeMath::max(0, 255.0 * (b - 0.333)),
                                            cweeMath::max(0, 255.0 * (b - 0.666)),
                                            14.0
                                        );
                                    }
                                    for (float& hilbertPos : policy) {
                                        try {
                                            AUTO pos_x = coord_to_x->HilbertPositionToValue(hilbertPos);
                                            AUTO pos_y = coord_to_y->HilbertPositionToValue(hilbertPos);

                                            UI_MapIcon icon = UI_MapIcon(pos_x, pos_y);
                                            icon.color = c;
                                            icon.size = 8;
                                            icon.HideOnCollision = false;
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        catch (...) {}
                                    }
                                }
                                bv_final["Policies"] = chaiscript::var(std::move(layer));
                            }
#endif
                        }
                        catch (...) {}

                        fileSystem->removeFile(fp);

                        return bv_final;
                    };
                    lib->add(chaiscript::fun([=](cweeSharedPtr<EPAnetProject>& control_project, int numValves) { return lambda(control_project, numValves, "PSO", 10 + 10 * numValves * numValves); }), "Optimize");
                    lib->add(chaiscript::fun([=](cweeSharedPtr<EPAnetProject>& control_project, int numValves, cweeStr method) { return lambda(control_project, numValves, method, 10 + 10 * numValves * numValves); }), "Optimize");
                    lib->add(chaiscript::fun([=](cweeSharedPtr<EPAnetProject>& control_project, int numValves, cweeStr method, int numPolicies) { return lambda(control_project, numValves, method, numPolicies); }), "Optimize");
                }

                /* Easy Map Display */ if (1) {
                    lib->add(chaiscript::fun([](cweeSharedPtr<EPAnetProject>& p) { using namespace cwee_units;
                        UI_Grid entireDisplay;
                        entireDisplay.RowDefinitions = { "Auto", "*", };
                        entireDisplay.ColumnDefinitions = { "*", "Auto", };

                        AUTO mapRegen = [](UI_Map& map, cweeSharedPtr<EPAnetProject> p, cweeStr option) {
                            using namespace cwee_units;
                            map.Layers.clear();
                            AUTO network = p->epanetProj->network;

                            switch (option.Hash()) {
                            default:
                            case cweeStr::Hash("Zone"):
                            {
                                foot_t minEl = std::numeric_limits<foot_t>::max();
                                foot_t maxEl = -std::numeric_limits<foot_t>::max();
                                for (auto& zone : network->Zone) {
                                    AUTO el = foot_t(zone->AverageElevation());
                                    if (el > maxEl) { maxEl = el; }
                                    if (el < minEl) { minEl = el; }
                                }

                                for (auto& zone : network->Zone) {
                                    if (zone) {
                                        UI_MapLayer layer;
                                        auto c = UI_Color((254.0 * (zone->AverageElevation() - minEl) / (maxEl - minEl))(), cweeRandomFloat(128, 254), cweeRandomFloat(128, 254), 255);
                                        for (auto& node : zone->Node) {
                                            UI_MapIcon icon; {
                                                icon.color = c;
                                                icon.longitude = node->X;
                                                icon.latitude = node->Y;
                                                icon.size = (node->Type_p != asset_t::RESERVOIR) ? 4 : 26;

                                                UI_Grid iconTag;
                                                {
                                                    std::shared_ptr<UI_StackPanel> iconTagContent = std::make_shared<UI_StackPanel>();
                                                    iconTag.OnLoaded = var(fun([=]() {
                                                        iconTagContent->Children.Clear();
                                                        {
                                                            iconTagContent->AddChild(var(std::map<std::string, Boxed_Value>({
                                                                  {"Longitude", var(double(node->X))}
                                                                , {"Latitude", var(double(node->Y))}
                                                                , {"Elevation", var(foot_t(node->El))}
                                                                , {"Name", var(cweeStr(node->Name_p))}
                                                                , {"Type", var(cweeStr(node->Type_p))}
                                                            })));
                                                        }
                                                        {
                                                            AUTO headPat = node->GetValue<_HEAD_>();
                                                            if (headPat) {
                                                                // Head or Level
                                                                {
                                                                    if (node->Type_p == asset_t::RESERVOIR) {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Level")));
                                                                            patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*headPat) - node->El));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                    else {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Head")));
                                                                            patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*headPat)));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                }
                                                                // Pressure
                                                                {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        AUTO pat = cweeUnitValues::cweeUnitPattern(*headPat) - node->El;
                                                                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_ft_water);
                                                                        pat1 = pat;
                                                                        AUTO pat2 = cweeUnitValues::cweeUnitPattern(1_s, 1_psi);
                                                                        pat2 = pat1;

                                                                        patContainer.AddChild(var(cweeStr("Pressure")));
                                                                        patContainer.AddChild(var(std::move(pat2)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }

                                                            AUTO demandPat = node->GetValue<_DEMAND_>();
                                                            if (demandPat) {
                                                                // Demand
                                                                {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        patContainer.AddChild(var(cweeStr("Demand")));
                                                                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_gpm);
                                                                        pat1 = cweeUnitValues::cweeUnitPattern(*demandPat);
                                                                        patContainer.AddChild(var(std::move(pat1)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }
                                                        }
                                                        // iconTagContent->Update();
                                                    }));
                                                    iconTag.AddChild(var(iconTagContent), 0, 0, 1, 1);
                                                }
                                                icon.Tag = var(std::move(iconTag));

                                                icon.IconPathGeometry = node->Icon();
                                                icon.HideOnCollision = (node->Type_p != asset_t::RESERVOIR);
                                                if (node->Type_p == asset_t::RESERVOIR) {
                                                    icon.Label = "Reservoir " + node->Name_p;
                                                }
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        AUTO minPressureCustomer = zone->FindMinPressureCustomer();
                                        if (minPressureCustomer) {
                                            UI_MapIcon icon; {
                                                icon.color = UI_Color(254, 25, 25, 254);
                                                icon.longitude = minPressureCustomer->X;
                                                icon.latitude = minPressureCustomer->Y;
                                                icon.size = 16;
                                                AUTO headPat = *minPressureCustomer->GetValue<_HEAD_>() - minPressureCustomer->El;
                                                AUTO avgPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetAvgValue()());
                                                AUTO minPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetMinValue()());

                                                UI_Grid iconTag;
                                                {
                                                    std::shared_ptr<UI_StackPanel> iconTagContent = std::make_shared<UI_StackPanel>();
                                                    iconTag.OnLoaded = var(fun([=]() {
                                                        iconTagContent->Children.Clear();
                                                        {
                                                            iconTagContent->AddChild(var(std::map<std::string, Boxed_Value>({
                                                                  {"Avg Pressure", var(std::move(avgPressure))}
                                                                , {"Min Pressure", var(std::move(minPressure))}
                                                                , {"Longitude", var(double(minPressureCustomer->X))}
                                                                , {"Latitude", var(double(minPressureCustomer->Y))}
                                                                , {"Elevation", var(foot_t(minPressureCustomer->El))}
                                                                , {"Name", var(cweeStr(minPressureCustomer->Name_p))}
                                                                , {"Type", var(cweeStr(minPressureCustomer->Type_p))}
                                                            })));
                                                        }
                                                        {
                                                            AUTO headPat = minPressureCustomer->GetValue<_HEAD_>();
                                                            if (headPat) {
                                                                // Head or Level
                                                                {
                                                                    if (minPressureCustomer->Type_p == asset_t::RESERVOIR) {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Level")));
                                                                            patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*headPat) - minPressureCustomer->El));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                    else {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Head")));
                                                                            patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*headPat)));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                }
                                                                // Pressure
                                                                {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        AUTO pat = cweeUnitValues::cweeUnitPattern(*headPat) - minPressureCustomer->El;
                                                                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_ft_water);
                                                                        pat1 = pat;
                                                                        AUTO pat2 = cweeUnitValues::cweeUnitPattern(1_s, 1_psi);
                                                                        pat2 = pat1;

                                                                        patContainer.AddChild(var(cweeStr("Pressure")));
                                                                        patContainer.AddChild(var(std::move(pat2)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }

                                                            AUTO demandPat = minPressureCustomer->GetValue<_DEMAND_>();
                                                            if (demandPat) {
                                                                // Demand
                                                                {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        patContainer.AddChild(var(cweeStr("Demand")));

                                                                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_gpm);
                                                                        pat1 = cweeUnitValues::cweeUnitPattern(*demandPat);
                                                                        patContainer.AddChild(var(std::move(pat1)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }
                                                        }
                                                        // iconTagContent->Update();
                                                        }));
                                                    iconTag.AddChild(var(iconTagContent), 0, 0, 1, 1);
                                                }
                                                icon.Tag = var(std::move(iconTag));

                                                icon.IconPathGeometry = minPressureCustomer->Icon();
                                                icon.HideOnCollision = false;
                                                icon.Label = cweeUnitValues::unit_value(minPressure).ToString();
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        for (auto& link : zone->Within_Link) {
                                            UI_MapPolyline line; {
                                                line.thickness = 3;
                                                line.color = UI_Color(c.R, c.G, c.B, 128);
                                                line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                                if (link->Vertices) {
                                                    for (auto& vert : link->Vertices->Array) {
                                                        line.AddPoint(vert.first, vert.second);
                                                    }
                                                }
                                                line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                                line.Tag = var(std::map<std::string, Boxed_Value>({
                                                    {"Name", var(cweeStr(link->Name_p))}
                                                    , {"Type", var(cweeStr(link->Type_p))}
                                                    }));
                                            }
                                            if (link->Icon().Length() > 0) {
                                                line.thickness = 4;
                                                UI_MapIcon icon; {
                                                    icon.color = c;
                                                    icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                    icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                    icon.size = 24;

                                                    UI_Grid iconTag;
                                                    {
                                                        std::shared_ptr<UI_StackPanel> iconTagContent = std::make_shared<UI_StackPanel>();
                                                        iconTag.OnLoaded = var(fun([=]() {
                                                            iconTagContent->Children.Clear();
                                                            {
                                                                iconTagContent->AddChild(var(std::map<std::string, Boxed_Value>({
                                                                    {"Longitude", var(double(icon.longitude))}
                                                                    , {"Latitude", var(double(icon.latitude))}
                                                                    , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                                    , {"Name", var(cweeStr(link->Name_p))}
                                                                    , {"Type", var(cweeStr(link->Type_p))}
                                                                    })));
                                                            }
                                                            {
                                                                AUTO pat = link->GetValue<_FLOW_>();
                                                                if (pat) {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        patContainer.AddChild(var(cweeStr("Flow")));

                                                                        AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_gpm);
                                                                        pat1 = cweeUnitValues::cweeUnitPattern(*pat);

                                                                        AUTO pattern = pat1;
                                                                        patContainer.AddChild(var(cweeStr::printf("Flow (%s)", cweeUnitValues::unit_value(pattern.RombergIntegral(pattern.GetMinTime(), pattern.GetMaxTime())).ToString().c_str())));

                                                                        patContainer.AddChild(var(std::move(pattern)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }
                                                            {
                                                                AUTO pat = link->GetValue<_HEADLOSS_>();
                                                                if (pat) {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        patContainer.AddChild(var(cweeStr("Headloss")));
                                                                        patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*pat)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }
                                                            {
                                                                AUTO pat = link->GetValue<_ENERGY_>();
                                                                if (pat) {
                                                                    UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                    {
                                                                        AUTO pattern = cweeUnitValues::cweeUnitPattern(*pat);
                                                                        patContainer.AddChild(var(cweeStr::printf("Energy (%s)", cweeUnitValues::kilowatt_hour(cweeUnitValues::unit_value(pattern.RombergIntegral(pattern.GetMinTime(), pattern.GetMaxTime()))).ToString().c_str())));
                                                                        patContainer.AddChild(var(std::move(pattern)));
                                                                    }
                                                                    iconTagContent->AddChild(var(std::move(patContainer)));
                                                                }
                                                            }
                                                            // iconTagContent->Update();
                                                            }));
                                                        iconTag.AddChild(var(iconTagContent), 0, 0, 1, 1);
                                                    }
                                                    icon.Tag = var(std::move(iconTag));





                                                    icon.IconPathGeometry = link->Icon();
                                                    icon.HideOnCollision = false;
                                                    if (link->Type_p != asset_t::PIPE) {
                                                        icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                    }
                                                }
                                                layer.Children.push_back(var(std::move(icon)));
                                            }
                                            layer.Children.push_back(var(std::move(line)));
                                        }
                                        for (auto& link_dir : zone->Boundary_Link) {
                                            if (link_dir.second == ::epanet::direction_t::FLOW_IN_DMA) {
                                                auto& link = link_dir.first;
                                                UI_MapPolyline line; {
                                                    line.thickness = 3;
                                                    line.color = UI_Color(c.R, c.G, c.B, 128);
                                                    line.dashed = true;
                                                    line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                                    if (link->Vertices) {
                                                        for (auto& vert : link->Vertices->Array) {
                                                            line.AddPoint(vert.first, vert.second);
                                                        }
                                                    }
                                                    line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                                    line.Tag = var(std::map<std::string, Boxed_Value>({
                                                        {"Name", var(cweeStr(link->Name_p))}
                                                        , {"Type", var(cweeStr(link->Type_p))}
                                                        }));
                                                }
                                                if (link->Icon().Length() > 0) {
                                                    line.thickness = 4;
                                                    UI_MapIcon icon; {
                                                        icon.color = c;
                                                        icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                        icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                        icon.size = 24;

                                                        UI_Grid iconTag;
                                                        {
                                                            std::shared_ptr<UI_StackPanel> iconTagContent = std::make_shared<UI_StackPanel>();
                                                            iconTag.OnLoaded = var(fun([=]() {
                                                                iconTagContent->Children.Clear();
                                                                {
                                                                    iconTagContent->AddChild(var(std::map<std::string, Boxed_Value>({
                                                                        {"Longitude", var(double(icon.longitude))}
                                                                        , {"Latitude", var(double(icon.latitude))}
                                                                        , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                                        , {"Name", var(cweeStr(link->Name_p))}
                                                                        , {"Type", var(cweeStr(link->Type_p))}
                                                                        })));
                                                                }
                                                                {
                                                                    AUTO pat = link->GetValue<_FLOW_>();
                                                                    if (pat) {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Flow")));

                                                                            AUTO pat1 = cweeUnitValues::cweeUnitPattern(1_s, 1_gpm);
                                                                            pat1 = cweeUnitValues::cweeUnitPattern(*pat);

                                                                            AUTO pattern = pat1;
                                                                            patContainer.AddChild(var(cweeStr::printf("Flow (%s)", cweeUnitValues::unit_value(pattern.RombergIntegral(pattern.GetMinTime(), pattern.GetMaxTime())).ToString().c_str())));

                                                                            patContainer.AddChild(var(std::move(pattern)));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                }
                                                                {
                                                                    AUTO pat = link->GetValue<_HEADLOSS_>();
                                                                    if (pat) {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            patContainer.AddChild(var(cweeStr("Headloss")));
                                                                            patContainer.AddChild(var(cweeUnitValues::cweeUnitPattern(*pat)));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                }
                                                                {
                                                                    AUTO pat = link->GetValue<_ENERGY_>();
                                                                    if (pat) {
                                                                        UI_StackPanel patContainer; patContainer.MinHeight = 180; patContainer.MinWidth = 400;
                                                                        {
                                                                            AUTO pattern = cweeUnitValues::cweeUnitPattern(*pat);
                                                                            patContainer.AddChild(var(cweeStr::printf("Energy (%s)", cweeUnitValues::kilowatt_hour(cweeUnitValues::unit_value(pattern.RombergIntegral(pattern.GetMinTime(), pattern.GetMaxTime()))).ToString().c_str())));
                                                                            patContainer.AddChild(var(std::move(pattern)));
                                                                        }
                                                                        iconTagContent->AddChild(var(std::move(patContainer)));
                                                                    }
                                                                }
                                                                // iconTagContent->Update();
                                                            }));
                                                            iconTag.AddChild(var(iconTagContent), 0, 0, 1, 1);
                                                        }
                                                        icon.Tag = var(std::move(iconTag));

                                                        icon.IconPathGeometry = link->Icon();
                                                        icon.HideOnCollision = false;
                                                        if (link->Type_p != asset_t::PIPE) {
                                                            icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                        }
                                                    }
                                                    layer.Children.push_back(var(std::move(icon)));
                                                }
                                                layer.Children.push_back(var(std::move(line)));
                                            }
                                        }

                                        layer.Name = zone->Name_p;

                                        map.Layers.push_back(var(std::move(layer)));
                                    }
                                }
                            }
                            break;
                            case cweeStr::Hash("Pressure"):
                            {
                                pounds_per_square_inch_t minEl = std::numeric_limits<pounds_per_square_inch_t>::max();
                                pounds_per_square_inch_t maxEl = -std::numeric_limits<pounds_per_square_inch_t>::max();
                                for (auto& zone : network->Zone) {
                                    AUTO el = zone->MinimumNodePressure();
                                    if (el > maxEl) { maxEl = el; }
                                    if (el < minEl) { minEl = el; }

                                    el = zone->MaximumNodePressure();
                                    if (el > maxEl) { maxEl = el; }
                                    if (el < minEl) { minEl = el; }
                                }

                                for (auto& zone : network->Zone) {
                                    if (zone) {
                                        UI_MapLayer layer;
                                        
                                        for (auto& node : zone->Node) {
                                            UI_MapIcon icon; {
                                                auto cV = (254.0 * (node->GetAvgPressure() - minEl) / (maxEl - minEl))();
                                                auto c = UI_Color(cV, cV, cV, 255);

                                                icon.color = c;
                                                icon.longitude = node->X;
                                                icon.latitude = node->Y;
                                                icon.size = (node->Type_p != asset_t::RESERVOIR) ? 4 : 26;
                                                icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                    {"Longitude", var(double(node->X))}
                                                    , {"Latitude", var(double(node->Y))}
                                                    , {"Elevation", var(foot_t(node->El))}
                                                    , {"Name", var(cweeStr(node->Name_p))}
                                                    , {"Type", var(cweeStr(node->Type_p))}
                                                    }));
                                                icon.IconPathGeometry = node->Icon();
                                                icon.HideOnCollision = (node->Type_p != asset_t::RESERVOIR);
                                                if (node->Type_p == asset_t::RESERVOIR) {
                                                    icon.Label = "Reservoir " + node->Name_p;
                                                }
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        AUTO minPressureCustomer = zone->FindMinPressureCustomer();
                                        if (minPressureCustomer) {
                                            UI_MapIcon icon; {
                                                icon.color = UI_Color(254, 25, 25, 254);
                                                icon.longitude = minPressureCustomer->X;
                                                icon.latitude = minPressureCustomer->Y;
                                                icon.size = 16;
                                                AUTO headPat = *minPressureCustomer->GetValue<_HEAD_>() - minPressureCustomer->El;
                                                AUTO avgPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetAvgValue()());
                                                AUTO minPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetMinValue()());

                                                icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                      {"Avg Pressure", var(std::move(avgPressure))}
                                                    , {"Min Pressure", var(std::move(minPressure))}
                                                    , {"Longitude", var(double(minPressureCustomer->X))}
                                                    , {"Latitude", var(double(minPressureCustomer->Y))}
                                                    , {"Elevation", var(foot_t(minPressureCustomer->El))}
                                                    , {"Name", var(cweeStr(minPressureCustomer->Name_p))}
                                                    , {"Type", var(cweeStr(minPressureCustomer->Type_p))}
                                                    }));
                                                icon.IconPathGeometry = minPressureCustomer->Icon();
                                                icon.HideOnCollision = false;
                                                icon.Label = cweeUnitValues::unit_value(minPressure).ToString();
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        for (auto& link : zone->Within_Link) {
                                            auto cV = (254.0 * (((link->StartingNode->GetAvgPressure() + link->EndingNode->GetAvgPressure()) / 2.0) - minEl) / (maxEl - minEl))();
                                            auto c = UI_Color(cV, cV, cV, 255);
                                            UI_MapPolyline line; {
                                                line.thickness = 3;
                                                line.color = UI_Color(c.R, c.G, c.B, 128);
                                                line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                                if (link->Vertices) {
                                                    for (auto& vert : link->Vertices->Array) {
                                                        line.AddPoint(vert.first, vert.second);
                                                    }
                                                }
                                                line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                                line.Tag = var(std::map<std::string, Boxed_Value>({
                                                    {"Name", var(cweeStr(link->Name_p))}
                                                    , {"Type", var(cweeStr(link->Type_p))}
                                                    }));
                                            }
                                            if (link->Icon().Length() > 0) {
                                                line.thickness = 4;
                                                UI_MapIcon icon; {
                                                    icon.color = c;
                                                    icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                    icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                    icon.size = 24;
                                                    icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                        {"Longitude", var(double(icon.longitude))}
                                                        , {"Latitude", var(double(icon.latitude))}
                                                        , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                        , {"Name", var(cweeStr(link->Name_p))}
                                                        , {"Type", var(cweeStr(link->Type_p))}
                                                        }));
                                                    icon.IconPathGeometry = link->Icon();
                                                    icon.HideOnCollision = false;
                                                    if (link->Type_p != asset_t::PIPE) {
                                                        icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                    }
                                                }
                                                layer.Children.push_back(var(std::move(icon)));
                                            }
                                            layer.Children.push_back(var(std::move(line)));
                                        }
                                        for (auto& link_dir : zone->Boundary_Link) {
                                            if (link_dir.second == ::epanet::direction_t::FLOW_IN_DMA) {
                                                auto& link = link_dir.first;
                                                auto cV = (254.0 * (((link->StartingNode->GetAvgPressure() + link->EndingNode->GetAvgPressure()) / 2.0) - minEl) / (maxEl - minEl))();
                                                auto c = UI_Color(cV, cV, cV, 255);
                                                UI_MapPolyline line; {
                                                    line.thickness = 3;
                                                    line.color = UI_Color(c.R, c.G, c.B, 128);
                                                    line.dashed = true;
                                                    line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                                    if (link->Vertices) {
                                                        for (auto& vert : link->Vertices->Array) {
                                                            line.AddPoint(vert.first, vert.second);
                                                        }
                                                    }
                                                    line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                                    line.Tag = var(std::map<std::string, Boxed_Value>({
                                                        {"Name", var(cweeStr(link->Name_p))}
                                                        , {"Type", var(cweeStr(link->Type_p))}
                                                        }));
                                                }
                                                if (link->Icon().Length() > 0) {
                                                    line.thickness = 4;
                                                    UI_MapIcon icon; {
                                                        icon.color = c;
                                                        icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                        icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                        icon.size = 24;
                                                        icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                            {"Longitude", var(double(icon.longitude))}
                                                            , {"Latitude", var(double(icon.latitude))}
                                                            , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                            , {"Name", var(cweeStr(link->Name_p))}
                                                            , {"Type", var(cweeStr(link->Type_p))}
                                                            }));
                                                        icon.IconPathGeometry = link->Icon();
                                                        icon.HideOnCollision = false;
                                                        if (link->Type_p != asset_t::PIPE) {
                                                            icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                        }
                                                    }
                                                    layer.Children.push_back(var(std::move(icon)));
                                                }
                                                layer.Children.push_back(var(std::move(line)));
                                            }
                                        }

                                        layer.Name = zone->Name_p;

                                        map.Layers.push_back(var(std::move(layer)));
                                    }
                                }                            
                            }
                            break;
                            }
                            return map;
                        };
                        std::shared_ptr<UI_Map> map = std::make_shared<UI_Map>();

                        /* Headers / Controls */ {
                            UI_StackPanel header; 
                            header.Orientation = "Horizontal";
                            header.Spacing = 10;
                            {
                                UI_Button zoneDisplay; {
                                    zoneDisplay.Content = var(UI_TextBlock("Zone Display"));
                                    zoneDisplay.Clicked = var(fun([=]() {
                                        mapRegen(*map, p, "Zone");
                                        map->Update();
                                    }));
                                    header.AddChild(var(std::move(zoneDisplay)));
                                }
                                UI_Button pressureDisplay; {
                                    pressureDisplay.Content = var(UI_TextBlock("Pressure Display"));
                                    pressureDisplay.Clicked = var(fun([=]() {
                                        mapRegen(*map, p, "Pressure");
                                        map->Update();
                                    }));
                                    header.AddChild(var(std::move(pressureDisplay)));
                                }
                            }
                            entireDisplay.AddChild(var(std::move(header)), 0, 0, 1, 1);
                        }
                        /* Map */ {
                            mapRegen(*map, p, "Zone");
                            entireDisplay.AddChild(var(map), 1, 0, 1, 1);
                        }
                        /* Zone List */ {
                            UI_Grid zones;
                            zones.RowDefinitions = { "Auto", "1", "*", };
                            zones.RowSpacing = 5;
                            {
                                {
                                    zones.AddChild(var(UI_TextBlock("Zones")), 0, 0, 1, 1);
                                }
                                {
                                    UI_Rectangle rect; {
                                        rect.Fill = UI_Color(128, 128, 128, 255);
                                    }
                                    zones.AddChild(var(std::move(rect)), 1, 0, 1, 1);
                                }
                                {
                                    std::vector<Boxed_Value> zoneList; {
                                        for (auto& zone : p->epanetProj->network->Zone) {
                                            if (zone) {
                                                UI_Button zoneDisplay; {
                                                    std::shared_ptr<UI_TextBlock> ZoneName = std::make_shared<UI_TextBlock>(zone->Name_p);
                                                    ZoneName->Foreground = UI_Color(64, 188, 64, 255);
                                                    ZoneName->HorizontalAlignment = "Center";
                                                    ZoneName->HorizontalTextAlignment = "Center";
                                                    zoneDisplay.Content = var(ZoneName);
                                                    cweeSharedPtr<bool> visibilityTracker = make_cwee_shared<bool>(true);                                                    
                                                    zoneDisplay.Clicked = var(fun([=]() {
                                                        for (auto& layer : map->Layers) {
                                                            auto* layerP = boxed_cast<UI_MapLayer*>(layer);
                                                            if (layerP && layerP->Name == zone->Name_p) {
                                                                *visibilityTracker = !*visibilityTracker;
                                                                if (*visibilityTracker) {
                                                                    ZoneName->Foreground = UI_Color(64, 188, 64, 255);
                                                                    ZoneName->Update();
                                                                }
                                                                else {
                                                                    ZoneName->Foreground = UI_Color(255, 128, 128, 255);
                                                                    ZoneName->Update();
                                                                }
                                                                layerP->SetVisibility(*visibilityTracker);
                                                            }
                                                        }
                                                    }));                                                    
                                                }
                                                zoneList.push_back(var(std::move(zoneDisplay)));
                                            }
                                        }

                                    }
                                    zones.AddChild(var(std::move(zoneList)), 2, 0, 1, 1);
                                }
                            }
                            entireDisplay.AddChild(var(std::move(zones)), 0, 1, 2, 1);
                        }                        
                        
                        return entireDisplay;
                    }), "Display");


                    AUTO lambda = [](UI_Map& map, cweeSharedPtr<EPAnetProject>& p) {
                        using namespace cwee_units;

                        map.Layers.clear();
                        AUTO network = p->epanetProj->network;

                        foot_t minEl = std::numeric_limits<foot_t>::max();
                        foot_t maxEl = -std::numeric_limits<foot_t>::max();
                        for (auto& zone : network->Zone) {
                            AUTO el = foot_t(zone->AverageElevation());
                            if (el > maxEl) { maxEl = el; }
                            if (el < minEl) { minEl = el; }
                        }
                        for (auto& zone : network->Zone) {
                            if (zone) {
                                UI_MapLayer layer;
                                auto c = UI_Color((254.0 * (zone->AverageElevation() - minEl) / (maxEl - minEl))(), cweeRandomFloat(128, 254), cweeRandomFloat(128, 254), 255);                                                               
                                for (auto& node : zone->Node) {
                                    UI_MapIcon icon; {
                                        icon.color = c;
                                        icon.longitude = node->X;
                                        icon.latitude = node->Y;
                                        icon.size = (node->Type_p != asset_t::RESERVOIR) ? 4 : 26;
                                        icon.Tag = var(std::map<std::string, Boxed_Value>({
                                            {"Longitude", var(double(node->X))}
                                            , {"Latitude", var(double(node->Y))}
                                            , {"Elevation", var(foot_t(node->El))}
                                            , {"Name", var(cweeStr(node->Name_p))}
                                            , {"Type", var(cweeStr(node->Type_p))}
                                            }));
                                        icon.IconPathGeometry = node->Icon();
                                        icon.HideOnCollision = (node->Type_p != asset_t::RESERVOIR);
                                        if (node->Type_p == asset_t::RESERVOIR) {
                                            icon.Label = "Reservoir " + node->Name_p;
                                        }
                                    }
                                    layer.Children.push_back(var(std::move(icon)));
                                }
                                AUTO minPressureCustomer = zone->FindMinPressureCustomer();
                                if (minPressureCustomer) {
                                    UI_MapIcon icon; {
                                        icon.color = UI_Color(254, 25, 25, 254);
                                        icon.longitude = minPressureCustomer->X;
                                        icon.latitude = minPressureCustomer->Y;
                                        icon.size = 16;
                                        AUTO headPat = *minPressureCustomer->GetValue<_HEAD_>() - minPressureCustomer->El;
                                        AUTO avgPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetAvgValue()());
                                        AUTO minPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(headPat.GetMinValue()());

                                        icon.Tag = var(std::map<std::string, Boxed_Value>({
                                              {"Avg Pressure", var(std::move(avgPressure))}
                                            , {"Min Pressure", var(std::move(minPressure))}
                                            , {"Longitude", var(double(minPressureCustomer->X))}
                                            , {"Latitude", var(double(minPressureCustomer->Y))}
                                            , {"Elevation", var(foot_t(minPressureCustomer->El))}
                                            , {"Name", var(cweeStr(minPressureCustomer->Name_p))}
                                            , {"Type", var(cweeStr(minPressureCustomer->Type_p))}
                                            }));
                                        icon.IconPathGeometry = minPressureCustomer->Icon();
                                        icon.HideOnCollision = false;
                                        icon.Label = cweeUnitValues::unit_value(minPressure).ToString();
                                    }
                                    layer.Children.push_back(var(std::move(icon)));
                                }
                                for (auto& link : zone->Within_Link) {
                                    UI_MapPolyline line; {
                                        line.thickness = 3;
                                        line.color = UI_Color(c.R, c.G, c.B, 128);
                                        line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                        if (link->Vertices) {
                                            for (auto& vert : link->Vertices->Array) {
                                                line.AddPoint(vert.first, vert.second);
                                            }
                                        }
                                        line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                        line.Tag = var(std::map<std::string, Boxed_Value>({
                                            {"Name", var(cweeStr(link->Name_p))}
                                            , {"Type", var(cweeStr(link->Type_p))}
                                            }));
                                    }
                                    if (link->Icon().Length() > 0) {
                                        line.thickness = 4;
                                        UI_MapIcon icon; {
                                            icon.color = c;
                                            icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                            icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                            icon.size = 24;
                                            icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                {"Longitude", var(double(icon.longitude))}
                                                , {"Latitude", var(double(icon.latitude))}
                                                , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                , {"Name", var(cweeStr(link->Name_p))}
                                                , {"Type", var(cweeStr(link->Type_p))}
                                                }));
                                            icon.IconPathGeometry = link->Icon();
                                            icon.HideOnCollision = false;
                                            if (link->Type_p != asset_t::PIPE) {
                                                icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                            }
                                        }
                                        layer.Children.push_back(var(std::move(icon)));
                                    }
                                    layer.Children.push_back(var(std::move(line)));
                                }
                                for (auto& link_dir : zone->Boundary_Link) {
                                    if (link_dir.second == ::epanet::direction_t::FLOW_IN_DMA) {
                                        auto& link = link_dir.first;
                                        UI_MapPolyline line; {
                                            line.thickness = 3;
                                            line.color = UI_Color(c.R, c.G, c.B, 128);
                                            line.dashed = true;
                                            line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                            if (link->Vertices) {
                                                for (auto& vert : link->Vertices->Array) {
                                                    line.AddPoint(vert.first, vert.second);
                                                }
                                            }
                                            line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                            line.Tag = var(std::map<std::string, Boxed_Value>({
                                                {"Name", var(cweeStr(link->Name_p))}
                                                , {"Type", var(cweeStr(link->Type_p))}
                                                }));
                                        }
                                        if (link->Icon().Length() > 0) {
                                            line.thickness = 4;
                                            UI_MapIcon icon; {
                                                icon.color = c;
                                                icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                icon.size = 24;
                                                icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                    {"Longitude", var(double(icon.longitude))}
                                                    , {"Latitude", var(double(icon.latitude))}
                                                    , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                    , {"Name", var(cweeStr(link->Name_p))}
                                                    , {"Type", var(cweeStr(link->Type_p))}
                                                    }));
                                                icon.IconPathGeometry = link->Icon();
                                                icon.HideOnCollision = false;
                                                if (link->Type_p != asset_t::PIPE) {
                                                    icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                }
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        layer.Children.push_back(var(std::move(line)));
                                    }
                                }
                                map.Layers.push_back(var(std::move(layer)));
                            }
                        }
                        return map;
                    };

                    lib->add(chaiscript::fun([=](cweeSharedPtr<EPAnetProject>& p) { UI_Map o; return lambda(o, p); }), "UI_Map");
                    lib->add(chaiscript::fun(lambda), "=");
                    lib->add(chaiscript::type_conversion<cweeSharedPtr<EPAnetProject>, UI_Map>([=](cweeSharedPtr<EPAnetProject> p) { UI_Map o; return lambda(o, p); }, nullptr));
                }



            }
#else
            if (1) {
                /* Spattern */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Spattern);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spattern, Comment);
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Spattern> const& a) { if (a) return cweeUnitValues::cweeUnitPattern(a->Pat); else return cweeUnitValues::cweeUnitPattern(); }), "Pat");
                }
                /* Sdemand */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Sdemand);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Sdemand, Base);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Sdemand, Pat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Sdemand, TimePat);
                }
                /* Senergy */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Senergy);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, TimeOnLine);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, Efficiency);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, KwHrsPerFlow);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, KwHrs);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, MaxKwatts);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, TotalCost);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, CurrentPower);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Senergy, CurrentEffic);
                }
                /* Ssource */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Ssource);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Ssource, Concentration);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Ssource, Pat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Ssource, TimePat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Ssource, Smass);
                }
                /* Svertices */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Svertices);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svertices, Array); // List<pair<SCALAR, SCALAR>>
                }
                /* Sasset */ {
                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(asset_t, asset_t);
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Sasset);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Sasset, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Sasset, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Sasset, Name_p);
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                    lib->add(chaiscript::fun([](::epanet::Sasset const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Sasset> const& a, u64 t) { return a->GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");
                }
                /* Snode */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Snode);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Snode, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, Name_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, X);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, Y);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, El);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, D);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, S);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, Ke);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Snode, Zone);

                    //AUTO thingy1 = dispatch::detail::function_signature(&::epanet::Snode::X);
                    //AUTO thingy2 = dispatch::detail::function_signature([](cweeSharedPtr<::epanet::Snode>& a) { if (a) return &a->X; else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); });
                    // AUTO thingy3 = dispatch::detail::function_signature(&::epanet::Snode);

                    // AUTO thingy4 = chaiscript::dispatch::detail::make_callable(&::epanet::Snode::X, dispatch::detail::function_signature(&::epanet::Snode::X));

                    // AddNamespacedClassMember(::epanet, Snode, Type);
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Snode const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a, u64 t) { return a->GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a, u64 t) { return a->GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a, u64 t) { return a->GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEAD_>())); }), "Head");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_DEMAND_>())); }), "Demand");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Snode> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_QUALITY_>())); }), "Quality");
                }
                /* Slink */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Slink);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Slink, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Name_p);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, StartingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, EndingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Diam);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Len);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Kc);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Km);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Kb);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Kw);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, R_FlowResistance);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Slink, Vertices);

                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Slink, Area);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Slink, IsBiDirectionalPipe);


                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Slink const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a, u64 t) { return a->GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_FLOW_>())); }), "Flow");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_ENERGY_>())); }), "Energy");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEADLOSS_>())); }), "Headloss");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_VELOCITY_>())); }), "Velocity");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_SETTING_>())); }), "Setting");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Slink> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_STATUS_>())); }), "Status");
                }
                /* Stank */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Stank);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Stank, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Name_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, X);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Y);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, El);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, D);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, S);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Ke);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Zone);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Node);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Diameter);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Stank, Area);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Hmin);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Hmax);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Kb);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, TimePat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Pat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Vcurve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, Vcurve_Actual);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, MixModel);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, V1frac);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Stank, CanOverflow);

                    // AddNamespacedClassMember(::epanet, Snode, Type);
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Stank const& a, u64 t) { return a.GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Stank> const& a, u64 t) { return a->GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Stank> const& a, u64 t) { return a->GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr<::epanet::Stank> const& a, u64 t) { return a->GetCurrentValue<_QUALITY_>(t); }), "GetQualityValue");
                    
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Stank> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEAD_>())); }), "Head");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Stank> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_DEMAND_>())); }), "Demand");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Stank> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_QUALITY_>())); }), "Quality");
                }
                /* Spump */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Spump);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Spump, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Name_p);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, StartingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, EndingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Diam);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Len);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Kc);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Km);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Kb);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Kw);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, R_FlowResistance);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Vertices);

                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Spump, Area);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Spump, IsBiDirectionalPipe);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Link);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Ptype);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Qmax);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Hmax);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, H0);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, R);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, N);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Hcurve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, Ecurve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, TimeUpat);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Spump, TimeEpat);
                    AddNamespacedClassMember_SupportSharedPtr_SpecializedName(::epanet, Spump, Energy, EnergySummary);

                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Spump const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a, u64 t) { return a->GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_FLOW_>())); }), "Flow");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_ENERGY_>())); }), "Energy");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEADLOSS_>())); }), "Headloss");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_VELOCITY_>())); }), "Velocity");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_SETTING_>())); }), "Setting");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Spump> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_STATUS_>())); }), "Status");
                }
                /* Svalve */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Svalve);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Svalve, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Name_p);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, StartingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, EndingNode);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Diam);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Len);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Kc);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Km);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Kb);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Kw);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, R_FlowResistance);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Rc);
                    // AddNamespacedClassMember(::epanet, Slink, Type);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Vertices);

                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Svalve, Area);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Svalve, IsBiDirectionalPipe);
                    // AddNamespacedClassFunction_SupportSharedPtr(::epanet, Svalve, energy_generation_potential);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, Link);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Svalve, ProducesElectricity);
                    AddNamespacedClassMember_SupportSharedPtr_SpecializedName(::epanet, Svalve, Energy, EnergySummary);

                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](::epanet::Svalve const& a, u64 t) { return a.GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_ENERGY_>(t); }), "GetEnergyValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_HEADLOSS_>(t); }), "GetHeadlossValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_VELOCITY_>(t); }), "GetVelocityValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_SETTING_>(t); }), "GetSettingValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a, u64 t) { return a->GetCurrentValue<_STATUS_>(t); }), "GetStatusValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_FLOW_>())); }), "Flow");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_ENERGY_>())); }), "Energy");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEADLOSS_>())); }), "Headloss");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_VELOCITY_>())); }), "Velocity");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_SETTING_>())); }), "Setting");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Svalve> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_STATUS_>())); }), "Status");
                }
                /* Szone */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Szone);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, Icon);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Type_p);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Name_p);

                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::zoneType_t, zoneType_t);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Type);

                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::illDefined_t, illDefined_t);

                    
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, IllDefined);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Node);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Within_Link);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Szone, Boundary_Link);

                    ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(::epanet::direction_t, direction_t);

                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, AverageElevation);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, HasWaterDemand);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, IsIllDefined);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, AverageNodePressure);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, MinimumNodePressure);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, AverageCustomerPressure);
                    AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, MinimumCustomerPressure);

                    // AddNamespacedClassFunction_SupportSharedPtr(::epanet, Szone, NumberOfBoundaryLinksBetweenZones);                

                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](::epanet::Szone const& a, u64 t) { return a.GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a, u64 t) { return a->GetCurrentValue<_HEAD_>(t); }), "GetHeadValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a, u64 t) { return a->GetCurrentValue<_DEMAND_>(t); }), "GetDemandValue");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a, u64 t) { return a->GetCurrentValue<_FLOW_>(t); }), "GetFlowValue");

                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_FLOW_>())); }), "Flow");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_HEAD_>())); }), "Head");
                    lib->add(chaiscript::fun([](cweeSharedPtr < ::epanet::Szone> const& a) { return cweeUnitValues::cweeUnitPattern(*(a->GetValue<_DEMAND_>())); }), "Demand");
                }

                // ...

                /* Network */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Network);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nnodes);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ntanks);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Njuncs);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nlinks);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npipes);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npumps);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nvalves);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ncontrols);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Nrules);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Npats);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Ncurves);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, System);

                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Asset);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Node);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Link);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Tank);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Pump);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Valve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Zone);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Pattern);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Curve);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Control);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Rule);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Network, Adjlist);
                }
                /* Project */ {
                    AddNamespacedClassTemplate_SupportSharedPtr(::epanet, Project);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, network);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, parser);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, times);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, report);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, outfile);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, hydraul);
                    AddNamespacedClassMember_SupportSharedPtr(::epanet, Project, quality);
                }

                /* EPAnet Wrapper */ {
                    {
                        lib->add(chaiscript::user_type<EPAnet_Shared>(), "EPAnet");
                        lib->add(chaiscript::fun([]()->EPAnet_Shared* { return &*EPAnet; }), "EPAnet");
                        lib->add(chaiscript::fun([](EPAnet_Shared& p)->cweeSharedPtr<EPAnetProject> { return p.createNewProject(); }), "createNewProject");
                        lib->add(chaiscript::fun([](EPAnet_Shared& p, cweeSharedPtr<EPAnetProject>& proj) -> cweeSharedPtr<EPAnetProject> { 
                            auto fp = fileSystem->createRandomFile(fileType_t::INP);
                            proj->saveINP(fp); // save to a temp file
                            auto out = p.createNewProject();
                            out->loadINP(fp);
                            return out;
                        }), "copyProject");
                    }
                    {
                        lib->add(chaiscript::user_type<cweeSharedPtr<EPAnetProject>>(), "EPAnetProject");
                        lib->add(fun([]() { return EPAnet->createNewProject(); }), "EPAnetProject");
                        lib->add(constructor<cweeSharedPtr<EPAnetProject>(const cweeSharedPtr<EPAnetProject>&)>(), "EPAnetProject");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& a, const cweeSharedPtr<EPAnetProject>& b) { a = b; return a; }), "=");

                        lib->add(fun([](cweeSharedPtr<EPAnetProject> const& a) { return &*a->epanetProj; }), "project"); // data member as function, accessible as member or function in script. (i.e: "auto proj = EPAnet.createNewProject(); return proj.project.network.Nnodes;")
                        lib->add(fun([](cweeSharedPtr<EPAnetProject> const& a)-> cweeSharedPtr<epanet::Network>& { return a->epanetProj->network; }), "network"); // easier access to underlying network


                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->initBuild("", "", 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile) { proj->initBuild(rptFile, "", 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile) { proj->initBuild(rptFile, outFile, 0, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile, int unitsType) { proj->initBuild(rptFile, outFile, unitsType, 0); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr rptFile, cweeStr outFile, int unitsType, int headLossType) { proj->initBuild(rptFile, outFile, unitsType, headLossType); }), "initBuild");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile, cweeStr filePathToRptFile) { proj->loadINP(filePathToInpFile, filePathToRptFile); }), "loadINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile) { proj->loadINP(filePathToInpFile); }), "loadINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->closeINP(); }), "closeINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr filePathToInpFile) { proj->saveINP(filePathToInpFile); }), "saveINP");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getTitle(); }), "getTitle");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, bool isNode, int epanetIndex) { return proj->getDescription(isNode, epanetIndex); }), "getDescription");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, bool isNode, int epanetIndex, cweeStr description) { proj->setDescription(isNode, epanetIndex, description); }), "setDescription");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int objectType) { return proj->getCount(objectType); }), "getCount");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int units) { proj->setFlowUnits(units); }), "setFlowUnits");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int qualType, cweeStr chemName, cweeStr chemUnits, cweeStr traceNode) { proj->setQualType(qualType, chemName, chemUnits, traceNode); }), "setQualType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int param, units::time::second_t value) { proj->setTimeParam(param, value); }), "setTimeParam");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int option, double value) { proj->setOption(option, value); }), "setOption");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int option) { return proj->getOption(option); }), "getOption");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int param) { return proj->getTimeParam(param); }), "getTimeParam");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getFlowUnits(); }), "getFlowUnits");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentSimTime(); }), "getCurrentSimTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentSimTimestep(); }), "getCurrentSimTimestep");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getCurrentError(); }), "getCurrentError");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->fullHydraulicSim(); }), "fullHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->startHydraulicSim(); }), "startHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcHydraulicsAtTime(); }), "calcHydraulicsAtTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcHydraulicTimestepToNextTime(); }), "calcHydraulicTimestepToNextTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->resetHydraulicSim(); }), "resetHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->endHydraulicSim(); }), "endHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->fullQualitySim(); }), "fullQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->startQualitySim(); }), "startQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcQualityAtTime(); }), "calcQualityAtTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->calcQualityTimestepToNextTime(); }), "calcQualityTimestepToNextTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->resetQualitySim(); }), "resetQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->endQualitySim(); }), "endQualitySim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, int nodeType) { return proj->addNode(name, nodeType); }), "addNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int actionCode) { proj->deleteNode(nodeIndex, actionCode); }), "deleteNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, cweeStr newid) { proj->setNodeID(nodeIndex, newid); }), "setNodeID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int property, double value) { proj->setNodeValue(nodeIndex, property, value); }), "setNodeValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double elev, double dmnd, cweeStr dmndpat) { proj->setJuncData(nodeIndex, elev, dmnd, dmndpat); }), "setJuncData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double elev, double initlvl, double minlvl, double maxlvl, double diam, double minvol, cweeStr volcurve) { proj->setTankData(nodeIndex, elev, initlvl, minlvl, maxlvl, diam, minvol, volcurve); }), "setTankData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double x, double y) { proj->setCoord(nodeIndex, x, y); }), "setCoord");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getNodeindex(id); }), "getNodeindex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNodeid(nodeIndex); }), "getNodeid");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNodetype(nodeIndex); }), "getNodetype");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int property) { return proj->getNodevalue(nodeIndex, property); }), "getNodevalue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getCoordX(nodeIndex); }), "getCoordX");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getCoordY(nodeIndex); }), "getCoordY");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, double baseDemand, cweeStr demandPattern, cweeStr demandName) { proj->addDemand(nodeIndex, baseDemand, demandPattern, demandName); }), "addDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { proj->deleteDemand(nodeIndex, demandIndex); }), "deleteDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex, double baseDemand) { proj->setBaseDemand(nodeIndex, demandIndex, baseDemand); }), "setBaseDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex, int patIndex) { proj->setDemandPattern(nodeIndex, demandIndex, patIndex); }), "setDemandPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIdx, cweeStr demandName) { proj->setDemandName(nodeIndex, demandIdx, demandName); }), "setDemandName");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, cweeStr demandName) { return proj->getDemandIndex(nodeIndex, demandName); }), "getDemandIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex) { return proj->getNumDemands(nodeIndex); }), "getNumDemands");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getBaseDemand(nodeIndex, demandIndex); }), "getBaseDemand");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getDemandPattern(nodeIndex, demandIndex); }), "getDemandPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int nodeIndex, int demandIndex) { return proj->getDemandName(nodeIndex, demandIndex); }), "getDemandName");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->ParseNetwork(); }), "ParseNetwork");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, int linkType, cweeStr fromNode, cweeStr toNode) { return proj->addLink(id, linkType, fromNode, toNode); }), "addLink");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int actionCode) { proj->deleteLink(index, actionCode); }), "deleteLink");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int property, double value) { proj->setLinkValue(index, property, value); }), "setLinkValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double length, double diam, double rough, double mloss) { proj->setPipeData(index, length, diam, rough, mloss); }), "setPipeData");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr newid) { proj->setLinkID(index, newid); }), "setLinkID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int* inout_index, int linkType, int actionCode) { proj->setLinkType(inout_index, linkType, actionCode); }), "setLinkType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int node1, int node2) { proj->setLinkNodes(index, node1, node2); }), "setLinkNodes");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkUpstreamNode(index); }), "getlinkUpstreamNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkDownstreamNode(index); }), "getlinkDownstreamNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int property) { return proj->getLinkValue(index, property); }), "getLinkValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getlinkIndex(id); }), "getlinkIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getlinkID(index); }), "getlinkID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getLinkType(index); }), "getLinkType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex) { return proj->getPumpType(linkIndex); }), "getPumpType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex) { return proj->getHeadCurveIndex(linkIndex); }), "getHeadCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int linkIndex, int curveIndex) { proj->setHeadCurveIndex(linkIndex, curveIndex); }), "setHeadCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { proj->addPattern(id); }), "addPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { proj->deletePattern(index); }), "deletePattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr id) { proj->setPatternID(index, id); }), "setPatternID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period, double value) { proj->setPatternValue(index, period, value); }), "setPatternValue");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double* values, int len) { proj->setPattern(index, values, len); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getPatternIndex(id); }), "getPatternIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPatternID(index); }), "getPatternID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPatternLen(index); }), "getPatternLen");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period) { return proj->getPatternValue(index, period); }), "getPatternValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getAveragePatternValue(index); }), "getAveragePatternValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumPatterns(); }), "getNumPatterns");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getPattern(index); }), "getPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeThreadedList<float> data) { proj->setPattern(index, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeThreadedList<std::pair<u64, float>> data) { proj->setPattern(index, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, cweeThreadedList<float> data) { proj->setPattern(id, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id, cweeThreadedList<std::pair<u64, float>> data) { proj->setPattern(id, data); }), "setPattern");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { proj->addCurve(id); }), "addCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { proj->deleteCurve(index); }), "deleteCurve");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double* xValues, double* yValues, int nPoints) { proj->setCurve(index, xValues, yValues, nPoints); }), "setCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, cweeStr id) { proj->setCurveID(index, id); }), "setCurveID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int curveIndex, int pointIndex, double x, double y) { proj->setCurveValue(curveIndex, pointIndex, x, y); }), "setCurveValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr id) { return proj->getCurveIndex(id); }), "getCurveIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveID(index); }), "getCurveID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, int period) { return proj->getCurveValue(index, period); }), "getCurveValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveLen(index); }), "getCurveLen");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurveType(index); }), "getCurveType");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumCurves(); }), "getNumCurves");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index) { return proj->getCurve(index); }), "getCurve");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumControls(); }), "getNumControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlIndex) { proj->deleteControl(controlIndex); }), "deleteControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->deleteAllControls(); }), "deleteAllControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlType, cweeStr linkName, float setting, cweeStr nodeName, float level) { return proj->addControl(controlType, linkName, setting, nodeName, level); }), "addControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr entireControl) { return proj->addControl(entireControl); }), "addControl");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlindex, int* type, int* linkIndex, double* setting, int* nodeIndex, double* level) { proj->getControl(controlindex, type, linkIndex, setting, nodeIndex, level); }), "getControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int controlindex) { return proj->getControl(controlindex); }), "getControl");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getNumRules(); }), "getNumRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr ruleName) { return proj->findRuleIndex(ruleName); }), "findRuleIndex");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getRuleID(ruleIndex); }), "getRuleID");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, cweeStr linkIndex, cweeStr linkValue, cweeStr ifTrueThenStatusOrSetting, cweeStr refObjectType, cweeStr refObjectIndex, cweeStr refObjectVariable, cweeStr ComparisonOperator, cweeStr refObjectStatusOrValue, double priority) { return proj->addRule(name, linkIndex, linkValue, ifTrueThenStatusOrSetting, refObjectType, refObjectIndex, refObjectVariable, ComparisonOperator, refObjectStatusOrValue, priority); }), "addRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr ruleName, cweeStr entireRule, bool returnIndex = true) { return proj->addRule(ruleName, entireRule, returnIndex = true); }), "addRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { proj->deleteRule(ruleIndex); }), "deleteRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { proj->deleteAllRules(); }), "deleteAllRules");
                        // lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int* nPremises = nullptr, int* nThenActions = nullptr, int* nElseActions = nullptr, double* priority = nullptr) { proj->getRuleSizes(ruleIndex, nPremises = nullptr, nThenActions = nullptr, nElseActions = nullptr, priority = nullptr); }), "getRuleSizes");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int premiseIndex, int premiseType, int objectType, int objectIndex, int objectVariable, int comparisonOp, int comparisonStatus, float comparisonValue) { proj->setRule_Premise(ruleIndex, premiseIndex, premiseType, objectType, objectIndex, objectVariable, comparisonOp, comparisonStatus, comparisonValue); }), "setRule_Premise");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int actionIndex, int linkIndex, int status, double setting) { proj->setRule_ElseAction(ruleIndex, actionIndex, linkIndex, status, setting); }), "setRule_ElseAction");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int index, double priority) { proj->setRule_Priority(index, priority); }), "setRule_Priority");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex, int actionIndex, int linkIndex, int status, double setting) { proj->setRule_ThenAction(ruleIndex, actionIndex, linkIndex, status, setting); }), "setRule_ThenAction");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getRule(ruleIndex); }), "getRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getRules(); }), "getRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int projectNum, cweeStr name, bool isNode) { return proj->getEPAnetIndexOfAsset(projectNum, name, isNode); }), "getEPAnetIndexOfAsset");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, asset_t type) { return proj->isNode(type); }), "isNode");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int EPAnetIndex, bool isNode, int prop) { return proj->getCurrentHydraulicValue(EPAnetIndex, isNode, prop); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int EPAnetIndex, value_t value) { return proj->getCurrentHydraulicValue(EPAnetIndex, value); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, value_t value, bool isNode) { return proj->getCurrentHydraulicValue(name, value, isNode); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, cweeStr name, value_t value) { return proj->getCurrentHydraulicValue(name, value); }), "getCurrentHydraulicValue");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, int ruleIndex) { return proj->getImpactedAssetsFromRule(ruleIndex); }), "getImpactedAssetsFromRule");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getImpactedAssetsFromRules(); }), "getImpactedAssetsFromRules");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj) { return proj->getImpactedAssetsFromControls(); }), "getImpactedAssetsFromControls");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& proj, Pattern* pat, cweeStr name, u64 start, u64 end, u64 stepSeconds) { return proj->InsertPattern(pat, name, start, end, stepSeconds); }), "InsertPattern");















                    }
                    {
                        lib->add(chaiscript::user_type<cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>>(), "EPAnetHydraulicSim");
                        lib->add(constructor<cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>(const cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>&)>(), "EPAnetHydraulicSim");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>& a, const cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation>& b) { a = b; return a; }), "=");

                        lib->add(fun([](cweeSharedPtr<EPAnetProject>& a) { return a->StartHydraulicSimulation(); }), "StartHydraulicSimulation");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a, cweeUnitValues::minute minutes) { a->SetTimestep(minutes()); }), "SetTimestep");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { return a->GetCurrentSimTime(); }), "GetCurrentSimTime");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { a->DoSteadyState(); }), "DoSteadyState");
                        lib->add(fun([](cweeSharedPtr<EPAnetProject::EPAnetHydraulicSimulation> const& a) { return a->ShouldContinueSimulation(); }), "ShouldContinueSimulation");
                    }
                }

                /* EPAnet optimization */ 
                {                    
                    lib->add(chaiscript::fun([](cweeSharedPtr<EPAnetProject>& control_project, int numValves) {
                            ParticleSwarm_OptimizationManagementTool<true> // Alternating_OptimizationManagementTool< Random_OptimizationManagementTool<true>, Genetic_OptimizationManagementTool<true>>
                                ramt(numValves, 10 + 10 * numValves * numValves);

                            const auto PenaltyValue = ramt.default_constraint();

                            cweeSharedPtr<cweeThreadedMap<std::string, double>>  cachedResults = make_cwee_shared<cweeThreadedMap<std::string, double>>();

                            units::flowrate::gallon_per_minute_t minFlowrate = 40;

                            // skeletonize the control project                         
                            // control_project->RemoveDeadEnds();

                            // simulate the control project
                            {
                                AUTO sim = control_project->StartHydraulicSimulation();
                                while (true) {
                                    sim->DoSteadyState();
                                    if (!sim->ShouldContinueSimulation()) break;
                                }
                            }
                            AUTO control_network = control_project->epanetProj->network;

                            auto fp = fileSystem->createRandomFile(fileType_t::INP);
                            control_project->saveINP(fp); // save to a temp file

                            units::energy::megawatt_hour_t control_tot;
                            units::energy::megawatt_hour_t control_tot_pump; {
                                for (auto& x : control_network->Pump) { if (x) { control_tot += x->Energy.KwHrs; control_tot_pump += x->Energy.KwHrs; } }
                                for (auto& x : control_network->Valve) { if (x) { control_tot += x->Energy.KwHrs; } }
                            }

                            // units::power::kilowatt_t minPowerProduction = 10;
                            units::pressure::pounds_per_square_inch_t minAllowedCustomerPSI = units::pressure::pounds_per_square_inch_t(40.0);
                            units::pressure::pounds_per_square_inch_t minAllowedPSI = units::pressure::pounds_per_square_inch_t(5.0);
                            units::pressure::pounds_per_square_inch_t minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                            units::pressure::pounds_per_square_inch_t avgCustomerPSI = 0;
                            int pressureCount = 0;

                            for (auto& zone : control_network->Zone) {
                                if (zone && zone->HasWaterDemand()) {
                                    for (auto& node : zone->Node) {
                                        if (node && node->HasWaterDemand()) {
                                            AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                            cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(avgCustomerPSI, thisPSI, pressureCount);
                                            minCustomerPSI = std::min(minCustomerPSI, thisPSI);
                                            // minAllowedCustomerPSI = std::min(minAllowedCustomerPSI, thisPSI);
                                        }
                                    }
                                }
                            }

                            int nLinks = control_network->Nlinks;

                            cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_index = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                            cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_x = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                            cweeSharedPtr < cweeInterpolatedMatrix<float>> coord_to_y = make_cwee_shared < cweeInterpolatedMatrix<float> >();
                            for (int i = 1; i < control_network->Link.size(); i++) {
                                AUTO link = control_network->Link[i];
                                if (link) {
                                    coord_to_index->AddValue(link->X()(), link->Y()(), i);
                                    coord_to_x->AddValue(link->X()(), link->Y()(), link->X()());
                                    coord_to_y->AddValue(link->X()(), link->Y()(), link->Y()());
                                }
                            }

                            constexpr int maxIterations = 1000; // 1000 iterations
                            constexpr float eps = 0.000001f;

                            using sharedObjType = ChaiscriptOptimizationObj; // cweeOptimizer::sharedClass;

                            cweeThreadedMap < std::string, units::pressure::pounds_per_square_inch_t > controlNodeToMinCustomerZonePressure;
                            /* average customer pressure */ units::pressure::pounds_per_square_inch_t controlCustomerPressure = 0; { int count = 0;
                            for (auto& zone : control_network->Zone) {
                                units::pressure::pounds_per_square_inch_t controlZonePressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t >::max(); 
                                units::pressure::pounds_per_square_inch_t controlZoneCustomerPressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t >::max(); 
                                if (zone) {
                                    for (auto& node : zone->Node) {
                                        if (node) {
                                            AUTO pat = node->GetValue<_HEAD_>();
                                            if (pat) {
                                                units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                                units::pressure::head_t head = head_ft();
                                                units::pressure::pounds_per_square_inch_t thisPressure = head;

                                                if (node->HasWaterDemand()) {
                                                    cweeMath::rollingAverageRef(controlCustomerPressure, thisPressure, count);
                                                    controlZoneCustomerPressure = std::min(controlZoneCustomerPressure, thisPressure);
                                                }
                                                controlZonePressure = std::min(controlZonePressure, thisPressure);
                                            }
                                        }
                                    }
                                    for (auto& node : zone->Node) {
                                        if (node) {
                                            if (node->HasWaterDemand()) {
                                                controlNodeToMinCustomerZonePressure.Emplace(node->Name_p.c_str(), controlZoneCustomerPressure);
                                            }
                                            else {
                                                controlNodeToMinCustomerZonePressure.Emplace(node->Name_p.c_str(), controlZonePressure);
                                            }
                                        }
                                    }
                                }
                            }
                            }

                            AUTO linkIsGoodCandidate = [=](::epanet::Plink link) -> bool {
                                if (link) {
                                    if (link->Type_p == asset_t::PUMP) { // cannot replace a pump
                                        return false;
                                    }
                                    if (link->Type >= ::epanet::LinkType::PRV) {
                                        // existing prv's and fcv's are decent options considering they already reduce pressure - question is are they open? 
                                        if (link->Kc == ::epanet::MISSING) {
                                            if ((::epanet::StatusType)(double)link->Status(control_project->epanetProj->times.GetSimStartTime()) == ::epanet::OPEN)
                                            {
                                                return true; // OPEN VALVE
                                            }
                                            if ((::epanet::StatusType)(double)link->Status(control_project->epanetProj->times.GetSimStartTime()) == ::epanet::CLOSED)
                                            {
                                                return false; // CLOSED VALVE
                                            }
                                        }
                                        return false; // UNKNOWN
                                    }

                                    AUTO flowPat = link->GetValue<_FLOW_>();
                                    // AUTO min_flow = flowPat->GetMinValue();
                                    AUTO max_flow = flowPat->GetAvgValue(); // was GetMaxValue
                                    if (units::math::fabs(max_flow) < minFlowrate) {
                                        return false; // not enough flow to warrant analysis
                                    }

                                    return true;
                                }
                                return false;
                            };
                            AUTO getNextGoodLink = [=](::epanet::Plink link) -> ::epanet::Plink {
                                if (link && linkIsGoodCandidate(link)) {
                                    if (link->GetValue<_FLOW_>()->GetAvgValue() > units::flowrate::gallon_per_minute_t(0)) { // positive direction
                                        AUTO links = control_project->getConnectedLinks(link->N1); // upstream
                                        links.RemoveFast(link);
                                        if (links.size() == 1 && links[0]) {
                                            if (linkIsGoodCandidate(links[0])) {
                                                // this connecting pipe appears to be a good candidate. Make sure we don't cause a loop
                                                // by finding pipes that connect to a node that happen to both be feeding it. I.e.:
                                                // -->--> * <--<--
                                                // We want:
                                                // -->--> * -->-->

                                                if (links[0]->GetDownstreamNode()->Name_p == link->StartingNode->Name_p) {
                                                    // good! 
                                                    return links[0];

                                                }
                                                else {
                                                    // bad!
                                                    return link;
                                                }
                                            }
                                            else {
                                                return link;
                                            }
                                        }
                                        else  {
                                            // this pipe connects to a tree -> unclear how to handle, so accept this pipe
                                            return link;
                                        }
                                    }
                                    else { // negative direction
                                        AUTO links = control_project->getConnectedLinks(link->N2); // downstream
                                        links.RemoveFast(link);
                                        if (links.size() == 1 && links[0]) {
                                            if (linkIsGoodCandidate(links[0])) {
                                                // this connecting pipe appears to be a good candidate. Make sure we don't cause a loop
                                                // by finding pipes that connect to a node that happen to both be feeding it. I.e.:
                                                // -->--> * <--<--
                                                // We want:
                                                // -->--> * -->-->

                                                if (links[0]->GetDownstreamNode()->Name_p == link->EndingNode->Name_p) {
                                                    // good!
                                                    return link;
                                                }
                                                else {
                                                    // bad!
                                                    return links[0];
                                                }
                                            }
                                            else {
                                                return link;
                                            }
                                        }
                                        else {
                                            // this pipe connects to a tree -> unclear how to handle, so accept this pipe
                                            return link;
                                        }

                                    }
                                }
                                return link;
                            };

                            AUTO createEPAnetProject = [=](cweeThreadedList<float> const& policy, cweeList<::epanet::Plink>& newLinks, cweeStr& policyStr)->cweeSharedPtr<EPAnetProject> {
                                AUTO p = EPAnet->createNewProject();
                                p->loadINP(fp);

                                policyStr.Clear();
                                newLinks.Clear();
                                cweeList<int> indexes;
                                cweeThreadedMap<std::string, units::flowrate::gallon_per_minute_t> averageFlowrateMap;
                                for (auto& hilbertPos : policy) {
                                    int link_index = cweeMath::Rint(coord_to_index->HilbertPositionToValue(hilbertPos));

                                    auto newLinkName = "NewLink" + cweeStr(cweeRandomInt(0, 10000));
                                    try {
                                        auto control_link2 = control_network->Link[link_index];
                                        if (control_link2) {
                                            auto control_link = control_link2;
                                            int preventLoop = 10;
                                            // move upstream/downstream to get the best link that starts this path
                                            while (control_link2 && control_link) {
                                                control_link2 = getNextGoodLink(control_link);
                                                if (control_link == control_link2) {
                                                    break;
                                                }
                                                else {
                                                    control_link = control_link2;
                                                }
                                                if (--preventLoop <= 0) break;
                                            }

                                            if (control_link) {
                                                // the control_link now determines the link_index;
                                                link_index = control_project->getlinkIndex(control_link->Name_p);

                                                // policyStr.AddToDelimiter(link_index, ", ");

                                                if (indexes.Find(link_index)) {
                                                    // cannot repeat indexes! Makes no sense.
                                                    continue; // return nullptr;
                                                }
                                                else {
                                                    indexes.Append(link_index);
                                                }

                                                AUTO flowPat = control_link->GetValue<_FLOW_>();
                                                if (flowPat) {
                                                    units::flowrate::gallon_per_minute_t avgFlow = flowPat->GetAvgValue();
                                                    if (avgFlow > units::flowrate::gallon_per_minute_t(0)) { // positive direction
                                                        AUTO downstreamPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((control_link->EndingNode->GetValue<_HEAD_>()->GetAvgValue() - control_link->EndingNode->El)());
                                                        auto linkID = p->addLink(
                                                            newLinkName,
                                                            3, // 3 = PRV, 7 = TCV
                                                            control_link->StartingNode->Name_p,
                                                            control_link->EndingNode->Name_p
                                                        ); // name, type, fromName, toName                                                
                                                        if (linkID > 0) {
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                                1.0 // true
                                                            );
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                                downstreamPSI()
                                                            );
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_DIAMETER,
                                                                ((units::length::inch_t)(control_link->Diam))()
                                                            );
                                                            // accept the vertices as-is. 
                                                            AUTO destLink = p->epanetProj->network->Link[linkID];
                                                            if (destLink) {
                                                                destLink->Vertices = p->epanetProj->network->Link[link_index]->Vertices; // the vertices are good as-is
                                                                newLinks.Append(destLink);
                                                            }
                                                        }
                                                        else {
                                                            // there are places that are bad for valves or invalid -- for example, direct connection to Reservoirs.
                                                            return nullptr;
                                                        }
                                                    }
                                                    else { // reverse direction
                                                        AUTO downstreamPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((control_link->StartingNode->GetValue<_HEAD_>()->GetAvgValue() - control_link->StartingNode->El)());
                                                        auto linkID = p->addLink(
                                                            newLinkName,
                                                            3, // 3 = PRV, 7 = TCV
                                                            control_link->EndingNode->Name_p,
                                                            control_link->StartingNode->Name_p
                                                        ); // name, type, fromName, toName
                                                        if (linkID > 0) {
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                                1.0 // true
                                                            );
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                                downstreamPSI()
                                                            );
                                                            p->setLinkValue(
                                                                linkID, // index
                                                                ::epanet::EN_LinkProperty::EN_DIAMETER,
                                                                ((units::length::inch_t)(control_link->Diam))()
                                                            );
                                                            AUTO thisLink = p->epanetProj->network->Link[link_index];
                                                            AUTO destLink = p->epanetProj->network->Link[linkID];
                                                            if (destLink && thisLink && thisLink->Vertices) {
                                                                // the vertices need to be reversed to make any sense
                                                                thisLink->Vertices->Array.Reverse();
                                                                // copies the list to the end, then fast-swaps the first with the last items and results 
                                                                // in a reversed list.
                                                                
                                                                destLink->Vertices = thisLink->Vertices;
                                                            }
                                                            if (destLink) {
                                                                newLinks.Append(destLink);
                                                            }
                                                        }
                                                        else {
                                                            // there are places that are bad for valves or invalid -- for example, direct connection to Reservoirs.
                                                            return nullptr;
                                                        }
                                                    }
                                                    averageFlowrateMap.Emplace(newLinkName.c_str(), units::math::fabs(avgFlow));
                                                    p->deleteLink(link_index, 0);
                                                }
                                            }
                                        }
                                    }
                                    catch (...) {}
                                }
                                indexes.Sort();
                                for (auto& index : indexes) {
                                    policyStr.AddToDelimiter(index, ", ");
                                }

                                /* Regenerate zones after changing pipes */
                                p->ParseNetwork();

                                /* Check if the network is physically possible. */
                                for (auto& zone : p->epanetProj->network->Zone) {
                                    if (zone && zone->IllDefined != ::epanet::illDefined_t::Okay) {
                                        return nullptr;
                                    }
                                }

                                /* Check if one of the valves are essentially useless */
                                for (auto& link : newLinks) {
                                    if (link && (link->StartingNode->Zone == link->EndingNode->Zone)) {
                                        return nullptr;
                                    }
                                }

                                // optimize the setting for the new turbines or valves to reduce pressure as much as possible                                
                                for (auto& link : newLinks) {
                                    if (link && link->EndingNode) {
                                        // for each of the new links, go through their downstream zone nodes
                                        AUTO downstream_zone = link->EndingNode->Zone;
                                        if (downstream_zone && downstream_zone->HasWaterDemand()) {
                                            units::pressure::pounds_per_square_inch_t minZonePressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                            units::pressure::pounds_per_square_inch_t minZoneCustomerPressure = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                            for (auto& node : downstream_zone->Node) {
                                                if (node) {
                                                    // find this node in the control network to retrieve its pressure                                                    
                                                    int index = control_project->getNodeindex(node->Name_p);
                                                    if (index > 0) {
                                                        AUTO thisPressure = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)(((units::length::foot_t)(control_network->Node[index]->GetValue<_HEAD_>()->GetMinValue() - control_network->Node[index]->El))());
                                                        if (node->HasWaterDemand()) {
                                                            if (minZonePressure > thisPressure) minZonePressure = thisPressure;
                                                            if (minZoneCustomerPressure > thisPressure) minZoneCustomerPressure = thisPressure;
                                                        }
                                                        else {
                                                            if (minZonePressure > thisPressure) minZonePressure = thisPressure;
                                                        }
                                                    }
                                                }
                                            }

                                            if (minZoneCustomerPressure > minAllowedCustomerPSI && minZonePressure > minAllowedPSI) {
                                                // there is room to further decrease the pressure, potentially.
                                                AUTO diff1 = minZoneCustomerPressure - minAllowedCustomerPSI; // i.e. 65psi - 40psi = 25psi
                                                AUTO diff2 = minZonePressure - minAllowedPSI; // i.e. 25psi - 5psi = 20psi
                                                units::pressure::pounds_per_square_inch_t diff = diff1 < diff2 ? diff1 : diff2; // i.e. 20psi

                                                // estimate the energy production of this valve:
                                                units::pressure::head_t H = diff;
                                                units::length::foot_t H_ft = H();
                                                AUTO f_ptr = averageFlowrateMap.TryGetPtr(link->Name_p.c_str());
                                                if (f_ptr) {
                                                    AUTO kW_potential = cweeEng::CentrifugalPumpEnergyDemand_kW(*f_ptr, H_ft, 131);
                                                    if (kW_potential < (units::power::kilowatt_t)(50)) {
                                                        // this valve likely does not produce enough energy to be meaningful or a worthwhile ERT. BUT it could still be a PRV!
                                                        p->setLinkValue(
                                                            p->getlinkIndex(link->Name_p), // index
                                                            ::epanet::EN_LinkProperty::EN_VALVE_ELEC, // produces electricity
                                                            0.0
                                                        );
                                                    }
                                                }

                                                if (diff > (units::pressure::pounds_per_square_inch_t)0) {
                                                    p->setLinkValue( // change the previous setting (whatever it was) by reducing it as much as we could
                                                        p->getlinkIndex(link->Name_p), // index
                                                        ::epanet::EN_LinkProperty::EN_INITSETTING, // initial setting
                                                        p->getLinkValue(p->getlinkIndex(link->Name_p), ::epanet::EN_LinkProperty::EN_INITSETTING) - diff()
                                                    );
                                                }
                                            }
                                            else {
                                                // there is no room to further reduce pressure -- this position was never going to work.

                                            }
                                        }

                                    }
                                }
                                return p;
                            };

                            AUTO evaluate = [=](cweeSharedPtr<EPAnetProject> proj, cweeList<::epanet::Plink> newLinks) -> cweeThreadedMap<std::string, cweeUnitValues::unit_value> {
                                using namespace cwee_units;
                                scalar_t penalty = 0;

                                /* simulation */ {
                                    proj->epanetProj->times.Dur /= 7.0;
                                    AUTO sim = proj->StartHydraulicSimulation();
                                    while (true) {
                                        sim->DoSteadyState(::epanet::HydraulicSimulationQuality::LOWRES);
                                        if (proj->getCurrentError() >= 1) {
                                            // bad simulation -- big penalty.
                                            penalty = PenaltyValue;
                                            break;
                                        }
                                        if (!sim->ShouldContinueSimulation()) break;
                                    }
                                    proj->epanetProj->times.Dur *= 7.0;
                                }

                                // Get valve definitions, and if more cost effective, fix the PRV/ERT distinction.
                                cweeList< cweeEng::LeakageModel::ValveDefinition > valves;
                                for (auto& link : newLinks) {
                                    if (link && link->Type_p == asset_t::VALVE) {
                                        cweeEng::LeakageModel::ValveDefinition valveDef;
                                        {
                                            valveDef.dia = link->Diam;
                                            {
                                                AUTO energy = link->GetValue<_ENERGY_>();
                                                if (energy) {
                                                    valveDef.E_Production_Weekly_Summer = units::math::fabs(energy->GetAvgValue());
                                                    valveDef.E_Production_Weekly_Winter = units::math::fabs(energy->GetAvgValue());
                                                }
                                                else {
                                                    valveDef.E_Production_Weekly_Summer = 0; // KW
                                                    valveDef.E_Production_Weekly_Winter = 0; // KW
                                                }
                                            }
                                            {
                                                AUTO flow = link->GetValue<_FLOW_>();
                                                if (flow) {
                                                    valveDef.Qbep = flow->GetAvgValue();
                                                }
                                                else {
                                                    valveDef.Qbep = 0;
                                                }
                                            }
                                            {
                                                AUTO headloss = link->GetValue<_HEADLOSS_>();
                                                if (headloss) {
                                                    valveDef.Hbep = units::math::fabs(headloss->GetAvgValue());
                                                }
                                                else {
                                                    valveDef.Hbep = 0;
                                                }
                                            }
                                        }

                                        // If a PRV would be more cost effective, do that instead
                                        // The hydraulics are identical, so it's just a quick comparison.
                                        {
                                            constexpr scalar_t N_PRV = 20.0;
                                            constexpr scalar_t w_prv = 0.025 / 12.0;
                                            constexpr scalar_t w_pat = 0.025 / 12;
                                            constexpr scalar_t N_PAT = 20.0;
                                            constexpr scalar_t w_E = 0.025 / 12;
                                            constexpr Dollar_per_kilowatt_hour_t Ce = 0.12; // $ / KWh

                                            Dollar_t PRV_Install_Cost; {
                                                AUTO C_PRV = cweeEng::LeakageModel::CostOfPRV(valveDef.dia, w_prv);
                                                PRV_Install_Cost = N_PRV * C_PRV;
                                            }
                                            Dollar_t PRV_OM_Cost; {
                                                PRV_OM_Cost = 0; // assumed 0;
                                            }

                                            Dollar_t ERT_Install_Cost = 0;
                                            Dollar_t ERT_OM_Cost = 0;
                                            {
                                                AUTO C_PAT = cweeEng::LeakageModel::Get_F_Given_P<4 * 12>(11913.91_USD * valveDef.Qbep() * std::sqrt(valveDef.Hbep()), w_pat); // # average, inflated from 2019 to 2023 dollars
                                                C_PAT += (1.0 - 0.26) * C_PAT / 0.26;
                                                ERT_Install_Cost = N_PAT * C_PAT;

                                                for (int i = 0; i <= 12 * 30; i++) {
                                                    ERT_OM_Cost += cweeEng::LeakageModel::Get_F_Given_P(C_PAT * (0.15 / 12), w_pat, i);
                                                };
                                            }

                                            Dollar_t ERT_Energy_Production_Benefits = 0;
                                            {
                                                kilowatt_hour_t E_Production_Monthly = ((valveDef.E_Production_Weekly_Summer + valveDef.E_Production_Weekly_Winter) / 2.0) * cweeEng::LeakageModel::month_t(1);
                                                for (int i = 0; i <= 12 * 30; i++) {
                                                    ERT_Energy_Production_Benefits += E_Production_Monthly * cweeEng::LeakageModel::Get_F_Given_P(Ce, w_E, i);
                                                };
                                            }

                                            if ((ERT_Install_Cost + ERT_OM_Cost - ERT_Energy_Production_Benefits) > (PRV_Install_Cost + PRV_OM_Cost)) {
                                                AUTO valveP = link.CastReference<::epanet::Svalve>();
                                                if (valveP) {
                                                    valveP->ProducesElectricity = false;
                                                    valveP->GetValue<_ENERGY_>()->operator*=(0);
                                                }
                                                valveDef.E_Production_Weekly_Summer = 0; // KW
                                                valveDef.E_Production_Weekly_Winter = 0; // KW
                                            }
                                        }

                                        //if (cweeEng::LeakageModel::IsPrvMoreEffective(valveDef)) {
                                        //    AUTO valveP = link.CastReference<::epanet::Svalve>();
                                        //    if (valveP) {
                                        //        valveP->ProducesElectricity = false;
                                        //        valveP->GetValue<_ENERGY_>()->operator*=(0);
                                        //    }
                                        //    valveDef.E_Production_Weekly_Summer = 0; // KW
                                        //    valveDef.E_Production_Weekly_Winter = 0; // KW
                                        //}

                                        valves.Append(valveDef);
                                    }
                                }

                                // pressure penalties
                                for (auto& zone : proj->epanetProj->network->Zone) {
                                    if (zone && zone->HasWaterDemand()) {
                                        for (auto& node : zone->Node) {
                                            if (node) {
                                                AUTO pat = node->GetValue<_HEAD_>();
                                                if (pat) {
                                                    units::pressure::pounds_per_square_inch_t thisPressure;
                                                    {
                                                        units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                                        units::pressure::head_t head = head_ft();
                                                        thisPressure = head;
                                                    }

                                                    AUTO minPressure = controlNodeToMinCustomerZonePressure.TryGetPtr(node->Name_p.c_str());
                                                    if (minPressure && thisPressure < minAllowedCustomerPSI) {
                                                        // We reduced pressure more than the original pressure (Possibly OK). Is it unacceptable though? 
                                                        if (thisPressure < minAllowedPSI) {
                                                            // no node should be below 5 psi
                                                            penalty += (minAllowedPSI - thisPressure)();
                                                        }
                                                        if (node->HasWaterDemand() && thisPressure < *minPressure) {
                                                            // no customer should recieve less than 40psi                                                            
                                                            penalty += (*minPressure - thisPressure)(); // minAllowedCustomerPSI - thisPressure
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // total energy used minus total energy generated
                                kilowatt_hour_t totalEnergyDemand = 0; {
                                    for (auto& link : proj->epanetProj->network->Link) {
                                        if (link) {
                                            AUTO pat = link->GetValue<_ENERGY_>();
                                            if (pat) {
                                                totalEnergyDemand += pat->RombergIntegral(pat->GetMinTime(), pat->GetMaxTime());
                                            }
                                        }
                                    }
                                }

                                // average water pressure
                                pounds_per_square_inch_t customerPressure = 0; { int count = 0;
                                for (auto& zone : proj->epanetProj->network->Zone) {
                                    if (zone && zone->HasWaterDemand()) {
                                        for (auto& node : zone->Node) {
                                            if (node && node->HasWaterDemand()) {
                                                AUTO pat = node->GetValue<_HEAD_>();
                                                if (pat) {
                                                    units::length::foot_t head_ft = pat->GetAvgValue() - node->El;
                                                    units::pressure::head_t head = head_ft();
                                                    units::pressure::pounds_per_square_inch_t thisPressure = head;

                                                    cweeMath::rollingAverageRef(customerPressure, thisPressure, count);
                                                }
                                            }
                                        }
                                    }
                                }
                                }

                                // reduction to average customer pressure
                                pounds_per_square_inch_t customerPressureDelta = controlCustomerPressure - customerPressure;

                                // total length of mains
                                mile_t milesPipe = 0;
                                for (auto& zone : proj->epanetProj->network->Zone) {
                                    if (!zone) continue;
                                    for (auto& link : zone->Within_Link) {
                                        if (!link) continue;
                                        milesPipe += link->Len;
                                    }
                                }

                                year_t surveyFreq = cweeEng::LeakageModel::IdealSurveyFrequency(valves, milesPipe,
                                    controlCustomerPressure,
                                    customerPressure,
                                    1.3, // # infrastructure condition factor
                                    1.1 // # infrastructure leakage index                                    
                                );
                                AUTO result = cweeEng::LeakageModel::LeakModelResults(valves, milesPipe, surveyFreq,
                                    controlCustomerPressure,
                                    customerPressure,
                                    1.3, // # infrastructure condition factor
                                    1.1 // # infrastructure leakage index
                                    );

                                // Final Performance Results
                                Dollar_t performance = 0; {
                                    performance += (Dollar_t)((result["Total Costs"] - result["Total Benefits"])() / (30.0 * 12.0));

                                    if (std::isnan((performance + (Dollar_t)((std::min(std::max(0.0, penalty()), (double)numValves) / (double)numValves) * PenaltyValue))())) {
                                        performance = PenaltyValue;
                                    }
                                    else {
                                        performance += (Dollar_t)((std::min(std::max(0.0, penalty()), (double)numValves) / (double)numValves) * PenaltyValue);
                                    }
                                }

                                cweeThreadedMap<std::string, cweeUnitValues::unit_value> results;
                                results.Emplace("Penalty", penalty);
                                results.Emplace("Energy Demand", totalEnergyDemand);
                                results.Emplace("Customer Pressure", customerPressure);
                                results.Emplace("Pressure Delta", customerPressureDelta);
                                results.Emplace("Performance", performance);
                                results.Emplace("Survey Freq", surveyFreq);
                                results.Emplace("Total Costs", result["Total Costs"]);
                                results.Emplace("Total Benefits", result["Total Benefits"]);
                                results.Emplace("Net Benefits", result["Net Benefits"]);

                                return results;
                            };

                            cweeSharedPtr<cweeThreadedList<float>> BestPolicy = make_cwee_shared<cweeThreadedList<float>>();
                            cweeSharedPtr<float> bestPerformance = std::numeric_limits<float>::max();

                            /* policies are positions along the hilbert curve. */
                            AUTO todo = [=](cweeThreadedList<float> const& policy)-> float {
                                double performance = 0.0;
                                performance = PenaltyValue;

                                cweeStr policyStr; {
                                    cweeList<int> sortedIndexes1;
                                    for (auto& hilbertPos : policy) {
                                        int link_index = cweeMath::Rint(coord_to_index->HilbertPositionToValue(hilbertPos));
                                        sortedIndexes1.Append(link_index);
                                    }
                                    sortedIndexes1.Sort();
                                    for (auto& link_index : sortedIndexes1) {
                                        policyStr.AddToDelimiter(link_index, ", ");
                                    }
                                }

                                /* Check if we've seen this suggested placement before */
                                AUTO cachedPerformance = cachedResults->TryGetPtr(policyStr.c_str());
                                if (cachedPerformance) {
                                    performance = *cachedPerformance;
                                    return performance;
                                }

                                /* Generate the new EPAnet model. NOTE that it may change the placement of the valves as an improvement to the suggestion. */
                                cweeList<::epanet::Plink> newLinks; cweeStr newPolicyStr;
                                AUTO p = createEPAnetProject(policy, newLinks, newPolicyStr);
                                if (!p || newLinks.size() != numValves) { // newPolicyStr is unusable in this state                                    
                                    performance = PenaltyValue;
                                    cachedResults->Emplace(policyStr.c_str(), performance);
                                    return performance;
                                }

                                /* Check if we've seen this revised valve placement before. */
                                AUTO cachedPerformance2 = cachedResults->TryGetPtr(newPolicyStr.c_str());
                                if (cachedPerformance2) {
                                    performance = *cachedPerformance2;
                                    cachedResults->Emplace(policyStr.c_str(), performance);
                                    return performance;
                                }

                                /* The placement appears to be valid. Let's try it */
                                AUTO sim_result = evaluate(p, newLinks);

                                performance = sim_result.TryGetPtr("Performance")->operator()();

                                units::pressure::pounds_per_square_inch_t this_minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                units::pressure::pounds_per_square_inch_t this_avgCustomerPSI = 0;
                                /* Evaluate water customer pressures */ {
                                    int this_pressureCount = 0;
                                    for (auto& zone : p->epanetProj->network->Zone) {
                                        if (zone && zone->HasWaterDemand()) {
                                            for (auto& node : zone->Node) {
                                                if (node && node->HasWaterDemand()) {
                                                    AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                                    cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(this_avgCustomerPSI, thisPSI, this_pressureCount);
                                                    this_minCustomerPSI = std::min(this_minCustomerPSI, thisPSI);
                                                }
                                            }
                                        }
                                    }

                                    if (this_minCustomerPSI < units::pressure::pounds_per_square_inch_t(0)) {
                                        // very bad customer pressure
                                        performance += (((units::pressure::pounds_per_square_inch_t(0) - this_minCustomerPSI) / units::pressure::pounds_per_square_inch_t(1)) * units::energy::megawatt_hour_t(5000))(); // 50MWh penalty per min pressure reduction
                                    }
                                    if (this_minCustomerPSI < minCustomerPSI) {
                                        // bad customer pressure
                                        performance += (((minCustomerPSI - this_minCustomerPSI) / units::pressure::pounds_per_square_inch_t(1)) * units::energy::megawatt_hour_t(500))(); // 50MWh penalty per min pressure reduction
                                    }
                                }

                                cachedResults->Emplace(policyStr.c_str(), performance);
                                cachedResults->Emplace(newPolicyStr.c_str(), performance);
                                return performance;
                            };
                            AUTO objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
                                return todo(policy);
                            };
                            AUTO isFinishedFunc = [=](sharedObjType& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformanceV) -> bool {
                                auto& i = shared.results.Alloc();
                                i.bestPolicy = bestPolicy;
                                i.bestPerformance = bestPerformanceV;

                                bestPerformance.Lock();
                                if (*bestPerformance.UnsafeGet() > bestPerformanceV) {
                                    *bestPerformance.UnsafeGet() = bestPerformanceV;

                                    BestPolicy.Lock();
                                    *BestPolicy.UnsafeGet() = bestPolicy;
                                    BestPolicy.Unlock();
                                }
                                bestPerformance.Unlock();

                                if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
                                else {
                                    // ensure the most recent 5 iterations are better than the 5 before them.
                                    int i = shared.results.Num() - 1;
                                    float perf_new = 0;
                                    float perf_old = 0;
                                    int count = 0;
                                    for (; i >= (shared.results.Num() - 5); i--) {
                                        cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
                                    }
                                    count = 0;
                                    for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
                                        cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
                                    }

                                    // test for convergence
                                    auto is_convergent = [=](double sum, double prev_sum)->bool {
                                        double epsilon = eps; // Tolerance
                                        return std::abs(sum - prev_sum) < epsilon;
                                    };
                                    
                                    if (is_convergent(perf_new, perf_old)) {
                                        shared.numIterationsFailedImprovement++;
                                        if (shared.numIterationsFailedImprovement > 20) // (maxIterations / 10)) // 20 iterations processed and we saw no further improvement. Unlikely to see improvement with another 20.                         
                                            return true;
                                        else
                                            return false;
                                    }
                                    shared.numIterationsFailedImprovement = 0;
                                    return false; // we have not flat-lined. continue.
                                }
                            };

                            cweeSharedPtr<sharedObjType> shared = make_cwee_shared<sharedObjType>();
                            for (int i = 0; i < numValves; i++) ramt.lower_constraints().Emplace((float)coord_to_index->MinHilbertPosition(), i);
                            for (int i = 0; i < numValves; i++) ramt.upper_constraints().Emplace((float)coord_to_index->MaxHilbertPosition(), i);
                            cweeJob toAwait = cweeOptimizer::run_optimization(shared, ramt, std::function(objFunc), std::function(isFinishedFunc), maxIterations);

                            toAwait.AwaitAll();

                            std::map<std::string, chaiscript::Boxed_Value> bv_final;
                            try {
                                auto& bestPolicy = *BestPolicy;
                                auto& bestPerf = *bestPerformance;
                                bv_final["performance"] = chaiscript::var((float)*bestPerformance); 
                                bv_final["iterations"] = chaiscript::var((int)(shared->results.Num()));

                                cweeStr policy_Str;
                                cweeList<::epanet::Plink> newLinks;
                                AUTO p = createEPAnetProject(bestPolicy, newLinks, policy_Str);
                                if (p) {
                                    /* Perform the sim */ { AUTO hyd = p->StartHydraulicSimulation();
                                    while (true) {
                                        hyd->DoSteadyState();
                                        if (!hyd->ShouldContinueSimulation()) {
                                            break;
                                        }
                                    } }

                                    auto evaluated = evaluate(p, newLinks);
                                    for (auto& r : evaluated) {
                                        bv_final[r.first] = chaiscript::var(cweeUnitValues::unit_value(*r.second));
                                    }

                                    units::pressure::pounds_per_square_inch_t this_minCustomerPSI = std::numeric_limits<units::pressure::pounds_per_square_inch_t>::max();
                                    units::pressure::pounds_per_square_inch_t this_avgCustomerPSI = 0;
                                    /* Evaluate water customer pressures */ {
                                        int this_pressureCount = 0;
                                        for (auto& zone : p->epanetProj->network->Zone) {
                                            if (zone && zone->HasWaterDemand()) {
                                                for (auto& node : zone->Node) {
                                                    if (node && node->HasWaterDemand()) {
                                                        AUTO thisPSI = (units::pressure::pounds_per_square_inch_t)(units::pressure::head_t)((node->GetValue<_HEAD_>()->GetMinValue() - node->El)());
                                                        cweeMath::rollingAverageRef<units::pressure::pounds_per_square_inch_t>(this_avgCustomerPSI, thisPSI, this_pressureCount);
                                                        this_minCustomerPSI = std::min(this_minCustomerPSI, thisPSI);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    bv_final["Avg Pressure"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(this_avgCustomerPSI)));
                                    bv_final["Min Pressure"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(this_minCustomerPSI)));
                                    bv_final["Avg Pressure Reduction"] = chaiscript::var(cweeUnitValues::unit_value(cweeUnitValues::pounds_per_square_inch(avgCustomerPSI - this_avgCustomerPSI)));
                                    bv_final["Project"] = chaiscript::var(cweeSharedPtr<EPAnetProject>(p));
                                }

#if 0
                                /* display the points we reviewed into a UI_MapLayer */ {
                                    UI_MapLayer layer;
                                    int policyN = 0;
                                    int Npolicies = shared->policies.size();
                                    int numPoliciesPerIteration = 12;
                                    UI_Color c;
                                    for (cweeList<float>& policy : shared->policies) {
                                        policyN++;
                                        double b = (double)policyN / (double)Npolicies;
                                        if (policyN % numPoliciesPerIteration == 0) {
                                            c = UI_Color(
                                                cweeMath::max(0, 255.0 * (b)),
                                                cweeMath::max(0, 255.0 * (b - 0.333)),
                                                cweeMath::max(0, 255.0 * (b - 0.666)),
                                                14.0
                                            );
                                        }
                                        for (float& hilbertPos : policy) {
                                            try {
                                                AUTO pos_x = coord_to_x->HilbertPositionToValue(hilbertPos);
                                                AUTO pos_y = coord_to_y->HilbertPositionToValue(hilbertPos);

                                                UI_MapIcon icon = UI_MapIcon(pos_x, pos_y);
                                                icon.color = c;
                                                icon.size = 8;
                                                icon.HideOnCollision = false;
                                                layer.Children.push_back(var(std::move(icon)));
                                            }
                                            catch (...) {}
                                        }
                                    }
                                    bv_final["Policies"] = chaiscript::var(std::move(layer));
                                }
#endif
                            }
                            catch (...) {}

                            fileSystem->removeFile(fp);

                            return bv_final;
                        }), "Optimize");                    
                }

                /* Easy Map Display */ if (1){
                    AUTO lambda = [](UI_Map& map, cweeSharedPtr<EPAnetProject>& p) {
                        using namespace cwee_units;

                        map.Layers.clear();
                        AUTO network = p->epanetProj->network;

                        foot_t minEl = std::numeric_limits<foot_t>::max();
                        foot_t maxEl = -std::numeric_limits<foot_t>::max();
                        for (auto& zone : network->Zone) {
                            AUTO el = foot_t(zone->AverageElevation());
                            if (el > maxEl) { maxEl = el; }
                            if (el < minEl) { minEl = el; }
                        }
                        for (auto& zone : network->Zone) {
                            if (zone) {
                                UI_MapLayer layer;
                                auto c = UI_Color((254.0 * (zone->AverageElevation() - minEl) / (maxEl - minEl))(), cweeRandomFloat(128, 254), cweeRandomFloat(128, 254), 255);
                                for (auto& node : zone->Node) {
                                    UI_MapIcon icon; {
                                        icon.color = c;
                                        icon.longitude = node->X;
                                        icon.latitude = node->Y;
                                        icon.size = (node->Type_p != asset_t::RESERVOIR) ? 4 : 26;
                                        icon.Tag = var(std::map<std::string, Boxed_Value>({
                                            {"Longitude", var(double(node->X))}
                                            , {"Latitude", var(double(node->Y))}
                                            , {"Elevation", var(foot_t(node->El))}
                                            , {"Name", var(cweeStr(node->Name_p))}
                                            , {"Type", var(cweeStr(node->Type_p))}
                                        }));
                                        icon.IconPathGeometry = node->Icon();
                                        icon.HideOnCollision = (node->Type_p != asset_t::RESERVOIR);
                                        if (node->Type_p == asset_t::RESERVOIR) {
                                            icon.Label = "Reservoir " + node->Name_p;
                                        }
                                    }
                                    layer.Children.push_back(var(std::move(icon)));
                                }
                                for (auto& link : zone->Within_Link) {
                                    UI_MapPolyline line; {
                                        line.thickness = 3;
                                        line.color = UI_Color(c.R, c.G, c.B, 128);
                                        line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                        if (link->Vertices) {
                                            for (auto& vert : link->Vertices->Array) {
                                                line.AddPoint(vert.first, vert.second);
                                            }
                                        }
                                        line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                        line.Tag = var(std::map<std::string, Boxed_Value>({
                                            {"Name", var(cweeStr(link->Name_p))}
                                            , {"Type", var(cweeStr(link->Type_p))}
                                        }));
                                    }
                                    if (link->Icon().Length() > 0) {
                                        line.thickness = 4;
                                        UI_MapIcon icon; {
                                            icon.color = c;
                                            icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                            icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                            icon.size = 24;
                                            icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                {"Longitude", var(double(icon.longitude))}
                                                , {"Latitude", var(double(icon.latitude))}
                                                , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                , {"Name", var(cweeStr(link->Name_p))}
                                                , {"Type", var(cweeStr(link->Type_p))}
                                                }));
                                            icon.IconPathGeometry = link->Icon();
                                            icon.HideOnCollision = false;
                                            if (link->Type_p != asset_t::PIPE) {
                                                icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                            }
                                        }
                                        layer.Children.push_back(var(std::move(icon)));
                                    }
                                    layer.Children.push_back(var(std::move(line)));
                                }
                                for (auto& link_dir : zone->Boundary_Link) {
                                    if (link_dir.second == ::epanet::direction_t::FLOW_IN_DMA) {
                                        auto& link = link_dir.first;
                                        UI_MapPolyline line; {
                                            line.thickness = 3;
                                            line.color = UI_Color(c.R, c.G, c.B, 128);
                                            line.dashed = true;
                                            line.AddPoint(link->StartingNode->X, link->StartingNode->Y);
                                            if (link->Vertices) {
                                                for (auto& vert : link->Vertices->Array) {
                                                    line.AddPoint(vert.first, vert.second);
                                                }
                                            }
                                            line.AddPoint(link->EndingNode->X, link->EndingNode->Y);
                                            line.Tag = var(std::map<std::string, Boxed_Value>({
                                                {"Name", var(cweeStr(link->Name_p))}
                                                , {"Type", var(cweeStr(link->Type_p))}
                                            }));
                                        }
                                        if (link->Icon().Length() > 0) {
                                            line.thickness = 4;
                                            UI_MapIcon icon; {
                                                icon.color = c;
                                                icon.longitude = link->X()(); // (link->EndingNode->X + link->StartingNode->X) / 2.0;
                                                icon.latitude = link->Y()(); // (link->EndingNode->Y + link->StartingNode->Y) / 2.0;
                                                icon.size = 24;
                                                icon.Tag = var(std::map<std::string, Boxed_Value>({
                                                    {"Longitude", var(double(icon.longitude))}
                                                    , {"Latitude", var(double(icon.latitude))}
                                                    , {"Elevation", var((link->EndingNode->El + link->StartingNode->El) / 2.0)}
                                                    , {"Name", var(cweeStr(link->Name_p))}
                                                    , {"Type", var(cweeStr(link->Type_p))}
                                                    }));
                                                icon.IconPathGeometry = link->Icon();
                                                icon.HideOnCollision = false;
                                                if (link->Type_p != asset_t::PIPE) {
                                                    icon.Label = cweeStr(link->Type_p.ToString()) + " " + link->Name_p;
                                                }
                                            }
                                            layer.Children.push_back(var(std::move(icon)));
                                        }
                                        layer.Children.push_back(var(std::move(line)));
                                    }
                                }
                                map.Layers.push_back(var(std::move(layer)));
                            }
                        }
                        return map;
                    };

                    lib->add(chaiscript::fun([=](cweeSharedPtr<EPAnetProject>& p) { UI_Map o; return lambda(o, p); }), "UI_Map");
                    lib->add(chaiscript::fun(lambda), "=");
                    lib->add(chaiscript::type_conversion<cweeSharedPtr<EPAnetProject>, UI_Map>([=](cweeSharedPtr<EPAnetProject> p) { UI_Map o; return lambda(o, p); }, nullptr));
                }
            }
#endif
#endif
            return lib;
        };
    };
}; // namespace chaiscript