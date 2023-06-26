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
#include "WaterWatch_Module_P3.h"
#include "cweeScheduler.h"
#include "AppLayerRequests.h" // queue for job-requests for processing by another system
#include "enum.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_3p3() {
            auto lib = chaiscript::make_shared<Module>();
#if 1
            // Support for UI tools and drawing
            if (1) {
                UI_App::AppendToScriptingLanguage(*lib);
                UI_Color::AppendToScriptingLanguage(*lib);
                UI_FrameworkElement::AppendToScriptingLanguage(*lib);
                UI_Rectangle::AppendToScriptingLanguage(*lib);
                UI_TextBlock::AppendToScriptingLanguage(*lib);
                UI_Image::AppendToScriptingLanguage(*lib);
                UI_WebView::AppendToScriptingLanguage(*lib);
                UI_Panel::AppendToScriptingLanguage(*lib);
                UI_Grid::AppendToScriptingLanguage(*lib);
                UI_StackPanel::AppendToScriptingLanguage(*lib);
                UI_Control::AppendToScriptingLanguage(*lib);
                UI_Button::AppendToScriptingLanguage(*lib);
                UI_CheckBox::AppendToScriptingLanguage(*lib);
                UI_Slider::AppendToScriptingLanguage(*lib);
                UI_ToggleSwitch::AppendToScriptingLanguage(*lib);
                UI_ListView::AppendToScriptingLanguage(*lib);
                UI_TabView::AppendToScriptingLanguage(*lib);
                UI_Expander::AppendToScriptingLanguage(*lib);
                UI_TextBox::AppendToScriptingLanguage(*lib);
                UI_Plot::AppendToScriptingLanguage(*lib);

                UI_MapElement::AppendToScriptingLanguage(*lib);
                UI_MapIcon::AppendToScriptingLanguage(*lib);
                UI_MapPolyline::AppendToScriptingLanguage(*lib);
                //UI_MapBackground::AppendToScriptingLanguage(*lib);
                UI_MapLayer::AppendToScriptingLanguage(*lib);
                UI_Map::AppendToScriptingLanguage(*lib);
            }

            // Handle the app functions.
            {
                lib->add(chaiscript::fun([](std::string method) {
                    return AppLayerRequests->Query(method.c_str());
                    }), "AppFunction");
                lib->add(chaiscript::fun([](std::string method, std::string argument) {
                    cweeStr arg = argument.c_str();
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { arg };
                    return AppLayerRequests->Query(method.c_str(), arguments);
                    }), "AppFunction");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()) };
                    return AppLayerRequests->Query(method.c_str(), arguments);
                    }), "AppFunction");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2, std::string argument3) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()), cweeStr(argument3.c_str()) };
                    return AppLayerRequests->Query(method.c_str(), arguments);
                    }), "AppFunction");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2, std::string argument3, std::string argument4) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()), cweeStr(argument3.c_str()), cweeStr(argument4.c_str()) };
                    return AppLayerRequests->Query(method.c_str(), arguments);
                    }), "AppFunction");

                lib->add(chaiscript::fun([](std::string method) {
                    AppLayerRequests->AddRequest(method.c_str());
                    }), "AppFunctionAsync");
                lib->add(chaiscript::fun([](std::string method, std::string argument) {
                    cweeStr arg = argument.c_str();
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { arg };
                    AppLayerRequests->AddRequest(method.c_str(), arguments);
                    }), "AppFunctionAsync");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()) };
                    AppLayerRequests->AddRequest(method.c_str(), arguments);
                    }), "AppFunctionAsync");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2, std::string argument3) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()), cweeStr(argument3.c_str()) };
                    AppLayerRequests->AddRequest(method.c_str(), arguments);
                    }), "AppFunctionAsync");
                lib->add(chaiscript::fun([](std::string method, std::string argument1, std::string argument2, std::string argument3, std::string argument4) {
                    cweeThreadedList<cweeStr> arguments;
                    arguments = { cweeStr(argument1.c_str()), cweeStr(argument2.c_str()), cweeStr(argument3.c_str()), cweeStr(argument4.c_str()) };
                    AppLayerRequests->AddRequest(method.c_str(), arguments);
                    }), "AppFunctionAsync");
            }

            // cwee optimizer
            if (1) {
                ChaiscriptOptimizations::AppendToScriptingLanguage(*lib);
                if (1) {
                    lib->eval(R"(                        
                        def FindUnknownVariablesInEquation(string equation) /* "x*x + 2*y / z" */ {
	                        int prevSize = -1;
	                        Vector unidentifiedObjs;
	                        while(prevSize != unidentifiedObjs.size()){
		                        prevSize = unidentifiedObjs.size();
		                        try{
			                        cweeStr prelude = ""; for (x:unidentifiedObjs){ prelude.AddToDelimiter("double ${x};", " "); }
			                        string newCommand = "{ ${prelude}; ${equation}; }";
			                        eval(newCommand);
		                        }catch(e){
			                        if (e.what().Find("Can not find object: ") >= 0){
				                        string varName = e.what().Split("Can not find object: ")[1].Replace("\"","");
				                        unidentifiedObjs.push_back_ref(varName);
				                        continue;
			                        }
		                        }
	                        }
	                        return unidentifiedObjs; // ["x", "y", "z"]
                        };
                        def Minimize(string equation, double minSearch, double maxSearch){
                            Vector varNames = FindUnknownVariablesInEquation(equation);
                            if (varNames.size() > 0){
                                auto sampleFunc = fun[equation, varNames](Vector vals){ // take a vector<double>, return double
                                    double out;
                                    cweeStr funcStr;
                                    for (int i = varNames.size() - 1; i >= 0; i--){
                                        funcStr.AddToDelimiter("double ${varNames[i]} = ${vals[i]};", " ");
                                    }                                   
                                    funcStr = "{ ${funcStr}; return (double)(${equation}); }";
                                    out = eval(funcStr);
                                    return out;
                                };
                                Vector lowBound;
                                Vector highBound;
                                for (x : varNames){
                                    lowBound.push_back_ref(minSearch);
                                    highBound.push_back_ref(maxSearch);
                                }
                    
                                return Minimize(sampleFunc, lowBound, highBound).continue_with(fun[varNames](resultVector){
                                    Map out;
                                    for (int i = 0; i < varNames.size(); i++){
                                        out[varNames[i]] := resultVector[i];
                                    }
                                    return out;
                                });
                             }else{
                                return Async(fun[](){ return ["":0.0]; });
                             }
                        };
                        def Maximize(string equation, double minSearch, double maxSearch){
                            Vector varNames = FindUnknownVariablesInEquation(equation);
                            if (varNames.size() > 0){
                                auto sampleFunc = fun[equation, varNames](Vector vals){ // take a vector<double>, return double
                                    double out;
                                    cweeStr funcStr;
                                    for (int i = varNames.size() - 1; i >= 0; i--){
                                        funcStr.AddToDelimiter("double ${varNames[i]} = ${vals[i]};", " ");
                                    }                                   
                                    funcStr = "{ ${funcStr}; return (double)(${equation}); }";
                                    out = eval(funcStr);
                                    return out;
                                };
                                Vector lowBound;
                                Vector highBound;
                                for (x : varNames){
                                    lowBound.push_back_ref(minSearch);
                                    highBound.push_back_ref(maxSearch);
                                }
                                return Maximize(sampleFunc, lowBound, highBound).continue_with(fun[varNames](resultVector){
                                    Map out;
                                    for (int i = 0; i < varNames.size(); i++){
                                        out[varNames[i]] := resultVector[i];
                                    }
                                    return out;
                                });
                           }else{
                                return Async(fun[](){ return ["":0.0]; });
                            }
                        };
                    )");
                }
            }
#endif
            return lib;
        };
    };
}; // namespace chaiscript