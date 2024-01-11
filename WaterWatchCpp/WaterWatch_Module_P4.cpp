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
#include "WaterWatch_Module_P4.h"

#include "cweeTime.h"
#include "cweeUnitedValue.h"
#include "../WindowsPlatformTools/WindowsPlatformTools.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_4() {
            auto lib = chaiscript::make_shared<Module>();

            // CPU and Memory Load
            if (1) {
                lib->add(chaiscript::user_type<WindowsPlatform::cweeCpuInfo_t>(), "cweeCpuInfo");
                lib->add(chaiscript::constructor<WindowsPlatform::cweeCpuInfo_t()>(), "cweeCpuInfo");
                lib->add(chaiscript::constructor<WindowsPlatform::cweeCpuInfo_t(const WindowsPlatform::cweeCpuInfo_t&)>(), "cweeCpuInfo");
                lib->add(chaiscript::fun([](WindowsPlatform::cweeCpuInfo_t& a, WindowsPlatform::cweeCpuInfo_t& b) { a = b; return a; }), "=");

                lib->add(chaiscript::fun([]() { return WindowsPlatform::GetCPUInfo(); }), "GetCPUInfo");
                lib->add(chaiscript::fun([]() { return WindowsPlatform::GetCPULoad(); }), "GetCPULoad");
                lib->add(chaiscript::fun([]() { return Computer_Usage().PercentMemoryUsed();; }), "GetMemoryLoad");
            }


#if 1
            // unit_value?
            if (1) {
                // attempt 2
                using namespace cweeUnitValues;
                using namespace chaiscript;

                lib->add(user_type<unit_value>(), "value");
                lib->add(constructor<unit_value()>(), "value");
                lib->add(constructor<unit_value(const unit_value&)>(), "value");
                lib->add(fun([](unit_value& a, const unit_value& b)->unit_value& { a = b; return a; }), "=");

                // Numbers to unit_value
                lib->add(constructor<unit_value(int)>(), "value");
                lib->add(constructor<unit_value(double)>(), "value");
                lib->add(constructor<unit_value(float)>(), "value");
                lib->add(constructor<unit_value(u64)>(), "value");
                lib->add(constructor<unit_value(long)>(), "value");
                lib->add(type_conversion<int, unit_value>([](const int& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<double, unit_value>([](const double& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<float, unit_value>([](const float& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<u64, unit_value>([](const u64& a)->unit_value { return a; }, nullptr));
                lib->add(type_conversion<long, unit_value>([](const long& a)->unit_value { return a; }, nullptr));
                lib->add(chaiscript::fun([](int const& a) -> unit_value { return a; }), "value");
                lib->add(chaiscript::fun([](double const& a) -> unit_value { return a; }), "value");
                lib->add(chaiscript::fun([](float const& a) -> unit_value { return a; }), "value");
                lib->add(chaiscript::fun([](u64 const& a) -> unit_value { return a; }), "value");
                lib->add(chaiscript::fun([](long const& a) -> unit_value { return a; }), "value");

                lib->add(fun([](unit_value& a, const int& b)->unit_value& { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const double& b)->unit_value& { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const float& b)->unit_value& { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const u64& b)->unit_value& { a = b; return a; }), "=");
                lib->add(fun([](unit_value& a, const long& b)->unit_value& { a = b; return a; }), "=");

                // unit_value to Numbers
                lib->add(type_conversion<unit_value, int>([](const unit_value& a)->int { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, double>([](const unit_value& a)->double { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, float>([](const unit_value& a)->float { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, u64>([](const unit_value& a)->u64 { return a(); }, nullptr));
                lib->add(type_conversion<unit_value, long>([](const unit_value& a)->long { return a(); }, nullptr));
                lib->add(chaiscript::fun([](unit_value const& a) -> int { return a(); }), "int");
                lib->add(chaiscript::fun([](unit_value const& a) -> double { return a(); }), "double");
                lib->add(chaiscript::fun([](unit_value const& a) -> float { return a(); }), "float");
                lib->add(chaiscript::fun([](unit_value const& a) -> u64 { return a(); }), "u64");
                lib->add(chaiscript::fun([](unit_value const& a) -> long { return a(); }), "long");

                lib->add(fun([](int& a, const unit_value& b)->int& { a = b(); return a; }), "=");
                lib->add(fun([](double& a, const unit_value& b)->double& { a = b(); return a; }), "=");
                lib->add(fun([](float& a, const unit_value& b)->float& { a = b(); return a; }), "=");
                lib->add(fun([](u64& a, const unit_value& b)->u64& { a = b(); return a; }), "=");
                lib->add(fun([](long& a, const unit_value& b)->long& { a = b(); return a; }), "=");

                // operators 
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a.pow(b); }), "^");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) -> unit_value& { return a.pow_value(b); }), "^=");
                lib->add(chaiscript::fun(&unit_value::Add), "+");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) -> unit_value& { a += b; return a; }), "+=");
                lib->add(chaiscript::fun(&unit_value::Sub), "-");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) -> unit_value& { a -= b; return a; }), "-=");
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a * b; }), "*");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) -> unit_value& { a *= b; return a; }), "*=");
                lib->add(chaiscript::fun([](const unit_value& a, const unit_value& b) { return a / b; }), "/");
                lib->add(chaiscript::fun([](unit_value& a, const unit_value& b) -> unit_value& { a /= b; return a; }), "/=");
                lib->add(chaiscript::fun([](const unit_value& a) -> cweeStr { return a.ToString().c_str(); }), "to_string");
                lib->add(type_conversion<unit_value, std::string>([](const unit_value& a)->std::string { return a.ToString().c_str(); }, nullptr));
                lib->add(type_conversion<unit_value, cweeStr>([](const unit_value& a)->cweeStr { return a.ToString().c_str(); }, nullptr));

                // Comparators
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a == b; }), "==");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a < b; }), "<");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a > b; }), ">");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a >= b; }), ">=");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a <= b; }), "<=");
                lib->add(fun([](unit_value& a, const unit_value& b)->bool {return a != b; }), "!=");

                // Other Operators
                lib->add(fun([](unit_value& a) { a--; return a; }), "--");
                lib->add(fun([](unit_value& a) { a++; return a; }), "++");
                lib->add(fun([](unit_value const& a, unit_value const& b) -> std::vector<Boxed_Value> {
                    std::vector<Boxed_Value> out;
                    out.push_back(Boxed_Value(a));
                    if (b > a) {
                        for (unit_value c = a + 1; c < b; c++) {
                            out.push_back(Boxed_Value(c));
                        }
                        out.push_back(Boxed_Value(b));
                    }
                    else if (b < a) {
                        for (unit_value c = a - 1; c > b; c--) {
                            out.push_back(Boxed_Value(c));
                        }
                        out.push_back(Boxed_Value(b));
                    }
                    return out;
                    }), "..");

                // Functions
                lib->add(fun([](const unit_value& a)->std::string { return a.Abbreviation(); }), "Abbreviation");
                lib->add(fun([](const unit_value& a)->std::string { return a.UnitName(); }), "UnitName");
                lib->add(fun([](const unit_value& a)->cweeStr { return a.ToString().c_str(); }), "ToString");

                // cout
                lib->add(chaiscript::fun([](const unit_value& str) { std::cout << str << std::endl; }), "cout");

                // Derived unit types
                {
#define AddUnit(Type)\
                    lib->add(chaiscript::fun([]() -> unit_value { \
                        unit_value toReturn; \
                        Type out; \
                        toReturn = out; \
                        return toReturn; \
                    }), #Type); \
                    lib->add(chaiscript::fun([](unit_value const& a) -> unit_value {  \
                        unit_value toReturn; \
                        Type F; \
                        F = a; \
                        toReturn = F; \
                        return toReturn; \
                    }), #Type); \
                    lib->add(chaiscript::postfix((cweeStr("_") + cweeStr(Type::specialized_abbreviation())).c_str(), #Type));

#define AddUnitWithMetricPrefixes(Type)\
	                AddUnit(Type); \
	                AddUnit(femto ## Type); \
	                AddUnit(pico ## Type); \
	                AddUnit(nano ## Type); \
	                AddUnit(micro ## Type); \
	                AddUnit(milli ## Type); \
	                AddUnit(centi ## Type); \
	                AddUnit(deci ## Type); \
	                AddUnit(deca ## Type); \
	                AddUnit(hecto ## Type); \
	                AddUnit(kilo ## Type); \
	                AddUnit(mega ## Type); \
	                AddUnit(giga ## Type); \
	                AddUnit(tera ## Type); \
	                AddUnit(peta ## Type)

                    AddUnitWithMetricPrefixes(meter);
                    AddUnit(foot);
                    AddUnit(inch);
                    AddUnit(mile);
                    AddUnit(nauticalMile);
                    AddUnit(astronicalUnit);
                    AddUnit(yard);
                    AddUnitWithMetricPrefixes(gram);
                    AddUnit(metric_ton);
                    AddUnit(pound);
                    AddUnit(long_ton);
                    AddUnit(short_ton);
                    AddUnit(stone);
                    AddUnit(ounce);
                    AddUnit(carat);
                    AddUnit(slug);
                    AddUnitWithMetricPrefixes(second);
                    AddUnit(minute);
                    AddUnit(hour);
                    AddUnit(day);
                    AddUnit(week);
                    AddUnit(year);
                    AddUnit(julian_year);
                    AddUnit(gregorian_year);
                    AddUnitWithMetricPrefixes(ampere);
                    AddUnit(Dollar);
                    AddUnit(MillionDollar);
                    AddUnitWithMetricPrefixes(hertz);
                    AddUnit(meters_per_second);
                    AddUnit(feet_per_second);
                    AddUnit(feet_per_minute);
                    AddUnit(feet_per_hour);
                    AddUnit(miles_per_hour);
                    AddUnit(kilometers_per_hour);
                    AddUnit(knot);
                    AddUnit(meters_per_second_squared);
                    AddUnit(feet_per_second_squared);
                    AddUnit(standard_gravity);
                    AddUnitWithMetricPrefixes(newton);
                    AddUnitWithMetricPrefixes(pound_f);
                    AddUnit(dyne);
                    AddUnit(kilopond);
                    AddUnit(poundal);
                    AddUnitWithMetricPrefixes(pascals);
                    AddUnitWithMetricPrefixes(bar);
                    AddUnit(atmosphere);
                    AddUnit(pounds_per_square_inch);
                    AddUnit(head);
                    AddUnit(torr);
                    AddUnit(coulomb); // WithMetricPrefixes
                    AddUnitWithMetricPrefixes(ampere_hour);
                    AddUnitWithMetricPrefixes(watt);
                    AddUnit(horsepower);
                    AddUnitWithMetricPrefixes(joule);
                    AddUnitWithMetricPrefixes(calorie);
                    AddUnitWithMetricPrefixes(watt_minute);
                    AddUnitWithMetricPrefixes(watt_hour);
                    AddUnit(watt_day);
                    AddUnit(british_thermal_unit);
                    AddUnit(british_thermal_unit_iso);
                    AddUnit(british_thermal_unit_59);
                    AddUnit(therm);
                    AddUnit(foot_pound);
                    AddUnitWithMetricPrefixes(volt);
                    AddUnitWithMetricPrefixes(ohm);
                    AddUnit(siemens); // WithMetricPrefixes
                    AddUnit(square_meter);
                    AddUnit(square_foot);
                    AddUnit(square_inch);
                    AddUnit(square_mile);
                    AddUnit(square_kilometer);
                    AddUnit(hectare);
                    AddUnit(acre);
                    AddUnit(cubic_meter);
                    AddUnit(cubic_millimeter);
                    AddUnit(cubic_kilometer);
                    AddUnitWithMetricPrefixes(liter);
                    AddUnit(cubic_inch);
                    AddUnit(cubic_foot);
                    AddUnit(cubic_yard);
                    AddUnit(cubic_mile);
                    AddUnitWithMetricPrefixes(gallon);
                    AddUnit(imperial_gallon);
                    AddUnit(million_gallon);
                    AddUnit(imperial_million_gallon);
                    AddUnit(acre_foot);
                    AddUnit(quart);
                    AddUnit(pint);
                    AddUnit(cup);
                    AddUnit(fluid_ounce);
                    AddUnit(barrel);
                    AddUnit(bushel);
                    AddUnit(cord);
                    AddUnit(tablespoon);
                    AddUnit(teaspoon);
                    AddUnit(pinch);
                    AddUnit(dash);
                    AddUnit(drop);
                    AddUnit(fifth);
                    AddUnit(dram);
                    AddUnit(gill);
                    AddUnit(peck);
                    AddUnit(sack);
                    AddUnit(shot);
                    AddUnit(strike);
                    AddUnitWithMetricPrefixes(gram_per_second);
                    AddUnit(metric_ton_per_second);
                    AddUnit(metric_ton_per_minute);
                    AddUnit(metric_ton_per_hour);
                    AddUnit(metric_ton_per_day);
                    AddUnit(metric_ton_per_year);
                    AddUnit(cubic_meter_per_second);
                    AddUnit(cubic_meter_per_hour);
                    AddUnit(cubic_meter_per_day);
                    AddUnit(cubic_millimeter_per_second);
                    AddUnitWithMetricPrefixes(liter_per_second);
                    AddUnit(liter_per_minute);
                    AddUnit(liter_per_day);
                    AddUnit(megaliter_per_day);
                    AddUnit(cubic_inch_per_second);
                    AddUnit(cubic_inch_per_hour);
                    AddUnit(cubic_foot_per_second);
                    AddUnit(cubic_foot_per_hour);
                    AddUnit(gallon_per_second);
                    AddUnit(gallon_per_minute);
                    AddUnit(gallon_per_hour);
                    AddUnit(gallon_per_day);
                    AddUnit(gallon_per_year);
                    AddUnit(million_gallon_per_second);
                    AddUnit(million_gallon_per_minute);
                    AddUnit(million_gallon_per_hour);
                    AddUnit(million_gallon_per_day);
                    AddUnit(million_gallon_per_year);
                    AddUnit(imperial_million_gallon_per_second);
                    AddUnit(imperial_million_gallon_per_minute);
                    AddUnit(imperial_million_gallon_per_hour);
                    AddUnit(imperial_million_gallon_per_day);
                    AddUnit(imperial_million_gallon_per_year);
                    AddUnit(acre_foot_per_second);
                    AddUnit(acre_foot_per_minute);
                    AddUnit(acre_foot_per_hour);
                    AddUnit(acre_foot_per_day);
                    AddUnit(acre_foot_per_year);
                    AddUnit(kilograms_per_cubic_meter);
                    AddUnit(grams_per_milliliter);
                    AddUnit(kilograms_per_liter);
                    AddUnit(ounces_per_cubic_foot);
                    AddUnit(ounces_per_cubic_inch);
                    AddUnit(ounces_per_gallon);
                    AddUnit(pounds_per_cubic_foot);
                    AddUnit(pounds_per_cubic_inch);
                    AddUnit(pounds_per_gallon);
                    AddUnit(slugs_per_cubic_foot);
                    AddUnit(Dollar_per_joule);
                    AddUnit(Dollar_per_kilowatt_hour);
                    AddUnit(Dollar_per_watt);
                    AddUnit(Dollar_per_kilowatt);
                    AddUnit(Dollar_per_cubic_meter);
                    AddUnit(Dollar_per_gallon);   
                }

                // support for castingto/from cweeTime
                lib->add(type_conversion<unit_value, cweeTime>([](const unit_value& a)->cweeTime { return cweeTime((u64)(double)(second)a); }, nullptr));
                lib->add(type_conversion<cweeTime, unit_value>([](const cweeTime& a)->unit_value { return (second)(u64)a; }, nullptr));
                lib->add(fun([](unit_value& a, const cweeTime& b) { a = (second)(u64)b; return a; }), "=");
                lib->add(fun([](cweeTime& a, const unit_value& b) { a = cweeTime((u64)(double)(second)b); return a; }), "=");
                lib->add(chaiscript::fun([](cweeTime const& a) -> unit_value { return (second)(u64)a; }), "value");
                lib->add(chaiscript::fun([](unit_value const& a) -> cweeTime { return cweeTime((u64)(double)(second)a); }), "cweeTime");
                lib->eval(R"(
                    def `-`(value o){ return o * -1.0; };
                )");


            }

            if (1) {
                lib->eval(R"(
                    // def to_string(Function f){ return f.get_description(); } // allow functions to return as strings

                    /* Allow support for null, nullptr, etc. */
			        def `=`(void a, void b) { return a; }
			        def NULL(){ return; }
			        global& null = NULL();
                    global& nullptr = NULL();
			        def `==`(a, void b) { try{ return a.is_var_null(); }catch(e){ return true; } }
			        def `==`(void b, a) { try{ return a.is_var_null(); }catch(e){ return true; } }
			        def `!=`(a, void b) { try{ return !a.is_var_null(); }catch(e){ return false; } }
			        def `!=`(void b, a) { try{ return !a.is_var_null(); }catch(e){ return false; } }
			        def `==`(void a, void b) : a.is_var_null() == b.is_var_null() { return true; }
			        def `==`(void a, void b) : a.is_var_null() != b.is_var_null() { return false; }
			        def `!=`(void a, void b) : a.is_var_null() == b.is_var_null() { return false; }
			        def `!=`(void a, void b) : a.is_var_null() != b.is_var_null() { return true; }
			        def to_number(a) { var x = double(); x = cweeStr(a.to_string()); return x; }
                    def to_string(x) : x == nullptr{
                            return "";
                    };
                    def to_string(Dynamic_Object x){
                            return x.get_attrs().to_string();
                    };
                )");
            }

            lib->add(chaiscript::fun<std::function<
                std::map<std::string, chaiscript::Boxed_Value>(const chaiscript::dispatch::Proxy_Function_Base*)
                >>([](
                    const chaiscript::dispatch::Proxy_Function_Base* func
                    ) {
                        std::map<std::string, chaiscript::Boxed_Value> out;

                        auto x = func->get_param_types();

                        out[std::string("arity")] = chaiscript::Boxed_Value(func->get_arity());
                        out[std::string("isAttributeFunction")] = chaiscript::Boxed_Value(func->is_attribute_function());
                        if (x.size() > 0) {
                            out[std::string("returns")] = chaiscript::Boxed_Value(x[0]);
                        }
                        if (x.size() > 1) {
                            std::vector<chaiscript::Boxed_Value> t;
                            for (int i = 1; i < x.size(); i++) {
                                t.push_back(chaiscript::Boxed_Value(x[i]));
                            }
                            out[std::string("params")] = var(std::move(t));
                        }
                        if (x.size() > 1) {
                            std::vector<chaiscript::Boxed_Value> t2;
                            for (int i = 1; i < x.size(); i++) {
                                std::string paramName = func->get_ParameterName(i - 1);
                                if (paramName.size() >= 1) {
                                    t2.push_back(var(std::string(paramName)));
                                }                                
                            }
                            out[std::string("paramNames")] = var(std::move(t2));
                        }
                        return out;
                    }), "get");
            lib->add(chaiscript::fun<std::function<
                std::string(const chaiscript::dispatch::Proxy_Function_Base*, std::string, chaiscript::small_vector<chaiscript::Boxed_Value>)
                >>([](
                    const chaiscript::dispatch::Proxy_Function_Base* func, std::string Name, chaiscript::small_vector<chaiscript::Boxed_Value> paramTypeNames
                    ) {
                        cweeStr funcName = Name.c_str();
                        cweeStr funcDesc = func->get_description().c_str();
                        cweeList<cweeStr> paramTypes;
                        for (auto& pt : paramTypeNames) {
                            paramTypes.Append(chaiscript::boxed_cast<cweeStr>(pt));
                        }
                        cweeStr out;

                        auto arity = func->get_arity();
                        auto isAttribute = func->is_attribute_function();
                        auto& x = paramTypes; //  func->get_param_types();

                        if (x.size() > 0) {
                            out += x[0];
                        }

                        if (funcName == "") {
                            if (isAttribute)
                            {
                                out += " = ";
                                for (int i = 1; i < x.size() && i < 2; i++) {
                                    out += x[i]; // do once
                                    out += ".";
                                }
                                out += "(";
                            }
                            else {
                                out += " = (";
                                for (int i = 1; i < x.size() && i < 2; i++) {
                                    out += x[i]; // do once
                                }
                            }
                            for (int i = 2; i < x.size(); i++) {
                                out.AddToDelimiter(
                                    x[i]
                                    , ", "
                                ); // do as many times as needed
                            }
                            out += ")";
                        }
                        else {
                            if (isAttribute)
                            {
                                out += " = ";
                                for (int i = 1; i < x.size() && i < 2; i++) {
                                    out += x[i]; // do once
                                    out += ".";
                                }
                                out += funcName + "(";
                            }
                            else {
                                out += " = " + funcName + "(";
                                for (int i = 1; i < x.size() && i < 2; i++) {
                                    out += x[i]; // do once
                                }
                            }
                            for (int i = 2; i < x.size(); i++) {
                                out.AddToDelimiter(
                                    x[i]
                                    , ", "); // do as many times as needed
                            }
                            out += ")";
                        }

                        //if (!funcDesc.IsEmpty()) {
                        //    out.AddToDelimiter(funcDesc, "\n");
                        //}

                        return std::string(out.c_str());
                    }), "to_string_impl");

            {
                lib->eval(R"(
                def join(Vector container, cweeStr delim) {
                  auto& retval = cweeStr("");
                  for (j : container){
                    retval.AddToDelimiter(to_string(j), delim);
                  }
                  return retval;
                };
                def join(Map container, cweeStr delim) {
                  auto& retval = cweeStr("");
                  for (j : container){
                    retval.AddToDelimiter(to_string(j), delim);
                  }
                  return retval;
                };
                # to_string for Vectors
                def to_string(Vector x) {
                  auto& toReturn = cweeStr("[");
                  toReturn += x.join(cweeStr(", "));
                  toReturn += cweeStr("]");
                  return toReturn;
                };
                def to_string(Map x) {
                  auto& toReturn = cweeStr("[");
                  toReturn += x.join(cweeStr(", "));
                  toReturn += cweeStr("]");
                  return toReturn;
                };
                def to_string(Function func){ 
                    auto v = Vector(); 
                    for (x : func.get_param_types()){
                        v.push_back_ref(cweeStr(x.name()));
                    }
                    return to_string_impl(func, func.get_name(), v); 
                };
			    def IsVector(x){
				    var& tn = x.type_name();
				    if (tn.iFind("Vector") > tn.iFind("Map")){
					    return true;
				    }
				    return false;
			    }
			    def IsMap(x){
				    var& tn = x.type_name();
				    if (tn.iFind("Map") > tn.iFind("Vector")){
					    return true;
				    }
				    return false;
			    }
			    def FindObject(string name){
				    __LOCK__{
					    for (item : get_all_objects()){
						    for (sub : item.second){
							    if (sub.first == name){
								    return sub.second;
							    }
						    }
					    }
				    }
				    return null;
			    }
			    def Objects(){ 
				    var& out = Map();
				    __LOCK__{
					    out["Globals"] := Globals();

					    //for (item : get_all_objects()){
					    //	var& tempMap = Map();
					    //	for (sub : item.second){
					    //		if ((!sub.second.is_type("Type_Info")) && (sub.second != null)){
					    //			tempMap[sub.first] := sub.second;
					    //		}
					    //	}
					    //	out[item.first] := tempMap;
					    //}
				    }
				    return out; 
			    }
			    def Globals(){ 
				    var& out = Map();
				    __LOCK__{
					    for (item : get_global_objects()){
						    if ((!item.second.is_type("Type_Info")) && (item.second != null)){
							    out[item.first] := item.second;
						    }
					    }
				    }
				    out;
			    }
			    def GetAllTypes(){ 
				    var& out = Map();
				    __LOCK__{
					    for (item : get_global_objects()){
						    if ((item.second.is_type("Type_Info")) && (item.second != null)){
							    out[item.first] := item.second;
						    }
					    }
				    }
				    out;
			    }
			    def Parse(string command){
				    parse(command).ListNodes();
			    };
			    def DoParse(cweeStr command, cweeStr filename){
				    ListNodes(parse(command, filename));
			    };
                def GetScriptIds(cweeStr code){
                    Map out;
                    for (x : Parse(code)){
	                    var& t := x["node"];
	                    if (t.identifier == "Id" && t.text.to_string != ""){
		                    out[t.text.to_string] ?= 0;
	                    }
                    }
                    out.keys;
                };
			    def Functions(){ 
				    var& out = Map();
				    __LOCK__{
					    var& constructors = Map();
					    var& systemFunctions = Map();
					    var& freeFunctions = Map();
					    var& generalFunctions = Map();
					    var& x = get_functions();
					    for (item : x){
						    var& z = item.second.get();
						    var& toSave = item.second;
						    if (item.first == z["returns"].to_string()){ // constructor -- even if variadic, we know what it is. 
							    constructors[item.first] := toSave;
							    continue;
						    } 
						    if (!z.contains("params") || z["arity"] == 0){
							    freeFunctions[item.first] := toSave;
							    continue;
						    }
						    //if (z["arity"] < 0){ // variadic i.e. we don't know what to do with them. 
						    //	continue;
						    //}
						    if ((z.contains("params")) && (z["params"].is_type("Vector"))) {
							    var mayBeSystemFunction = true;
							    for (sub : z["params"]){
								    if ( 	(sub.to_string() != "Object") && 
 									    (sub.to_string() != "Number")
								    ){
									    mayBeSystemFunction = false;
									    break;
								    } 
							    }
							    if (mayBeSystemFunction){
								    systemFunctions[item.first] := toSave;
								    continue;
							    }
						    }
		
						    generalFunctions[item.first] := toSave;
					    }
					    out["Constructors"] := constructors; // functions whose name matches their return type -- i.e. constructors. 
					    out["Free Functions"] := freeFunctions; // guarranteed to be free i.e. not accessible after a "." oepration
					    out["System Functions"] := systemFunctions; // non-free functions that operate on Object or Number types. 
					    out["General"] := generalFunctions; // guarranteed to not be 'free' i.e. accessible after a "." operation
				    } 
				    return out;  
			    }
			    def AllFunctions(){ 
				    var& out = Map();
				    __LOCK__{					
					    var& preventDoubleDip = Map();
					    var& x = get_all_functions(); var mayBeSystemFunction = true;
					    var& constructors = Vector(); constructors.reserve(size_t(x.size()));
					    var& systemFunctions = Vector(); systemFunctions.reserve(size_t(x.size()));
					    var& freeFunctions = Vector(); freeFunctions.reserve(size_t(x.size()));
					    var& generalFunctions = Vector(); generalFunctions.reserve(size_t(x.size()));
					    for (item : x){
						    var& str = item.first.to_string();
						    var& d = item.second;
						
						    if (!preventDoubleDip.contains(str)){ preventDoubleDip[str] := Vector(); }						
						    else if (preventDoubleDip[str].contains(d)){ continue; }
						    preventDoubleDip[str].push_back_ref(d);
					
						    var& result = Pair(str, d);
						    var& z = d.get();
						    if (str == z["returns"].to_string()){ // constructor -- even if variadic, we know what it is. 							
							    constructors.push_back_ref(result);
						    } 
						    else if (!z.contains("params") || z["arity"] == 0){
							    freeFunctions.push_back_ref(result);
						    }
						    else if ((z.contains("params")) && (z["params"].is_type("Vector"))) {
							    mayBeSystemFunction = true;
							    var& str2;							
							    for (sub : z["params"]) {
								    str2 := sub.to_string();
								    if (str2 != "Object" && str2 != "Number"){
									    mayBeSystemFunction = false;
									    break;
								    } 
							    }						
							    if (mayBeSystemFunction) {
								    systemFunctions.push_back_ref(result);
							    } else {
								    generalFunctions.push_back_ref(result);
							    }
						    } else {
							    generalFunctions.push_back_ref(result);
						    }								
					    }
					    out["Constructors"] := constructors; // functions whose name matches their return type -- i.e. constructors. 
					    out["Free Functions"] := freeFunctions; // guarranteed to be free i.e. not accessible after a "." oepration
					    out["System Functions"] := systemFunctions; // non-free functions that operate on Object or Number types. 
					    out["General"] := generalFunctions; // guarranteed to not be 'free' i.e. accessible after a "." operation

				    } 
				    return out;  
			    }
            )");
            }
#endif

            return lib;
        };
    };
}; // namespace chaiscript