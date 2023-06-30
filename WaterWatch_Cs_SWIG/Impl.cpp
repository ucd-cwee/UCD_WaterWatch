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
#include "../WaterWatchCpp/Precompiled.h"
#include "../WindowsPlatformTools/WindowsPlatformTools.h"
#include "../WaterWatchCpp/Clock.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/AppLayerRequests.h"
#include "../WaterWatchCpp/Toasts.h"
#include "../WaterWatchCpp/FileSystemH.h"
#include "../WaterWatchCpp/Geocoding.h"
#include "../WaterWatchCpp/MachineLearning.h"
#include "../WaterWatchCpp/cweeJob.h"
#include "../WaterWatchCpp/Chaiscript_WaterWatch_Module.h"
#include "../WaterWatchCpp/ExternData.h"

#include "Header.h"

#pragma region cweeDateTime
cweeDateTime::cweeDateTime() : unixTime(0), year(0), month(0), day(0), hour(0), minute(0), second(0), milliseconds(0) {};
cweeDateTime::cweeDateTime(double x) : unixTime(x), year(0), month(0), day(0), hour(0), minute(0), second(0), milliseconds(0) {
	cweeTime t(x);
	year = t.tm_year() + 1900;
	month = t.tm_mon() + 1;
	day = t.tm_mday();
	hour = t.tm_hour();
	minute = t.tm_min();
	second = t.tm_sec();
	milliseconds = t.tm_fractionalsec();
};
#pragma endregion

#pragma region AWAITER
bool Awaiter::IsFinished() { return *(this->isFinished_m); };
std::string Awaiter::Result() { if (this->data_m) return *this->data_m; return ""; };
#pragma endregion

#pragma region Shared Data
SharedTimeSeriesPattern::SharedTimeSeriesPattern() : index_p(external_data->CreatePattern()), deleteDataWhenScopeEnds_p(true) {};
SharedTimeSeriesPattern::SharedTimeSeriesPattern(int index, bool deleteDataWhenScopeEnds) : index_p(index), deleteDataWhenScopeEnds_p(deleteDataWhenScopeEnds) {};
SharedTimeSeriesPattern::~SharedTimeSeriesPattern() {
	if (deleteDataWhenScopeEnds_p) {
		external_data->DeletePattern(index_p);
	}
};
void    SharedTimeSeriesPattern::Clear() { external_data->ClearPattern(index_p); };
void    SharedTimeSeriesPattern::AppendData(double time, float value) { external_data->AppendData(index_p, time, value); };
float   SharedTimeSeriesPattern::GetValue(double time) { return (double)external_data->GetValue(index_p, time); };
int     SharedTimeSeriesPattern::Index() { return index_p; };
std::vector<Pair<double, double>> SharedTimeSeriesPattern::GetTimeSeries() {
	std::vector<Pair<double, double>> out;
	for (auto& x : external_data->GetPattern(index_p)) {
		out.push_back(Pair<double,double>((double)x.first, (double)x.second));
	}
	return out;
};
double  SharedTimeSeriesPattern::GetMinTime() { return external_data->GetPatternRef(index_p)->GetMinTime()(); };
double  SharedTimeSeriesPattern::GetMaxTime() { return external_data->GetPatternRef(index_p)->GetMaxTime()(); };
int     SharedTimeSeriesPattern::GetNumValues() { return external_data->GetPatternRef(index_p)->GetNumValues(); };
std::string SharedTimeSeriesPattern::X_Units() {
	return external_data->GetPatternRef(index_p)->X_Type().Abbreviation();
};
std::string SharedTimeSeriesPattern::Y_Units() {
	return external_data->GetPatternRef(index_p)->Y_Type().Abbreviation();
};


SharedString::SharedString() : index_p(external_data->CreateString()), deleteDataWhenScopeEnds_p(true) {};
SharedString::SharedString(int index, bool deleteDataWhenScopeEnds) : index_p(index), deleteDataWhenScopeEnds_p(deleteDataWhenScopeEnds) {};
SharedString::~SharedString() {
	if (deleteDataWhenScopeEnds_p) {
		external_data->DeleteString(index_p);
	}
};
void    SharedString::Set(std::string v) { external_data->AppendData(index_p, v.c_str()); };
std::string   SharedString::Get() { 
	auto* p = external_data->GetStringRef(index_p);
	if (p) {
		return std::string(p->c_str());
	}
	else {
		return std::string("");
	}
};
int     SharedString::Index() { return index_p; };
#pragma endregion


#pragma region ScriptObject
ScriptObject::ScriptObject(cweeSharedPtr<void> p/*, cweeSharedPtr<void>const& Engine*/) : boxedvalue(p) {};
ScriptObject::~ScriptObject() {
	boxedvalue = nullptr;
};
std::string ScriptObject::Cast_String() {
	cweeSharedPtr<chaiscript::Boxed_Value> boxed(boxedvalue, [](void* p) { return static_cast<chaiscript::Boxed_Value*>(p); });
	std::string out;
	AUTO val = chaiscript::boxed_cast<std::string*>(*boxed);
	if (val) {
		out = *val;
	}
	return out;
};
Color_Interop ScriptObject::Cast_Color() {
	cweeSharedPtr<chaiscript::Boxed_Value> boxed(boxedvalue, [](void* p) { return static_cast<chaiscript::Boxed_Value*>(p); });
	Color_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_Color*>(*boxed);
	if (val) {
		out.R = val->R;
		out.G = val->G;
		out.B = val->B;
		out.A = val->A;
	}
	return out;
};
MapIcon_Interop ScriptObject::Cast_MapIcon() {
	cweeSharedPtr<chaiscript::Boxed_Value> boxed(boxedvalue, [](void* p) { return static_cast<chaiscript::Boxed_Value*>(p); });
	MapIcon_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapIcon*>(*boxed);
	if (val) {
		out.color.R = val->color.R;
		out.color.G = val->color.G;
		out.color.B = val->color.B;
		out.color.A = val->color.A;
		out.HideOnCollision = val->HideOnCollision;
		out.IconPathGeometry = val->IconPathGeometry.c_str();
		out.Label = val->Label.c_str();
		out.size = val->size;
		out.longitude = val->longitude;
		out.latitude = val->latitude;
	}
	return out;
};
MapPolyline_Interop ScriptObject::Cast_MapPolyline() {
	cweeSharedPtr<chaiscript::Boxed_Value> boxed(boxedvalue, [](void* p) { return static_cast<chaiscript::Boxed_Value*>(p); });
	MapPolyline_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapPolyline*>(*boxed);
	if (val) {
		out.color.R = val->color.R;
		out.color.G = val->color.G;
		out.color.B = val->color.B;
		out.color.A = val->color.A;
		out.thickness = val->thickness;
		out.dashed = val->dashed;
		
		out.coordinates.reserve(val->coordinates.size() + 1);
		for (auto& x : val->coordinates) { out.coordinates.push_back(Pair<double,double>(x.first, x.second)); }
	}
	return out;
};
MapLayer_Interop ScriptObject::Cast_MapLayer() {
	cweeSharedPtr<chaiscript::Boxed_Value> boxed(boxedvalue, [](void* p) { return static_cast<chaiscript::Boxed_Value*>(p); });
	MapLayer_Interop out2;
	AUTO val2 = chaiscript::boxed_cast<chaiscript::UI_MapLayer*>(*boxed);
	if (val2) {
		for (int i = 0; i < val2->Children.size(); i++) {
			auto& bv = val2->Children[i];
			if (bv.is_type(chaiscript::user_type<chaiscript::UI_MapIcon>())) {
				AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapIcon*>(bv);
				if (val) {
					MapIcon_Interop& out = out2.icons[i];
					out.color.R = val->color.R;
					out.color.G = val->color.G;
					out.color.B = val->color.B;
					out.color.A = val->color.A;
					out.HideOnCollision = val->HideOnCollision;
					out.IconPathGeometry = val->IconPathGeometry.c_str();
					out.Label = val->Label.c_str();
					out.size = val->size;
					out.longitude = val->longitude;
					out.latitude = val->latitude;		
				}
			}
			else if (bv.is_type(chaiscript::user_type<chaiscript::UI_MapPolyline>())) {
				AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapPolyline*>(bv);
				if (val) {
					MapPolyline_Interop& out = out2.polylines[i];
					out.color.R = val->color.R;
					out.color.G = val->color.G;
					out.color.B = val->color.B;
					out.color.A = val->color.A;
					out.thickness = val->thickness;
					out.dashed = val->dashed;
					out.coordinates.reserve(val->coordinates.size() + 1);
					for (auto& x : val->coordinates) { out.coordinates.push_back(Pair<double, double>(x.first, x.second)); }					
				}
			}
		}
	}
	return out2;
};
#pragma endregion

#pragma region Script Engine
ScriptEngine::ScriptEngine() : ptr(cweeSharedPtr<void>(cweeSharedPtr< chaiscript::WaterWatch_ChaiScript>(new chaiscript::WaterWatch_ChaiScript()), [](void* p) { return p; })) {};
ScriptEngine::~ScriptEngine() {};
Awaiter    ScriptEngine::DoScriptAsync(std::string command) {
	Awaiter toReturn;

	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> enginePtr(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	cweeJob task1([](std::string const& cmd, cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine, Awaiter& place_results)-> std::string {
		cweeStr STR;

		try {
			auto bv = engine->eval(cmd.c_str(), chaiscript::Exception_Handler());
			if (!bv.get_type_info().is_void()) STR = cweeStr::printf("Retrieved Object of Type: '%s'.", bv.get_type_info().name());
			else  STR = "";

			if (!bv.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), bv);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = chaiscript::boxed_cast<cweeStr>(str);
					}
				}
				catch (...) {}
			}
		}
		catch (chaiscript::Boxed_Value e) {
			if (!e.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
				}
				catch (chaiscript::exception::eval_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
				}
				catch (chaiscript::exception::dispatch_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arity_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arithmetic_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::runtime_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::exception e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (...) {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		catch (chaiscript::exception::eval_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
		}
		catch (chaiscript::exception::dispatch_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arity_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arithmetic_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::runtime_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::exception e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (...) {}

		*place_results.data_m = STR.c_str();
		*place_results.isFinished_m = true;

		return "";
	}, command, enginePtr, toReturn);
	task1.AsyncInvoke();

	return toReturn;
};
std::string    ScriptEngine::DoScript(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });
	
	cweeStr STR;
	try {
		auto bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());

		auto typeName = engine->type_name(bv);
		if (!bv.get_type_info().is_void()) STR = cweeStr::printf("Retrieved Object of Type: '%s'.", typeName.c_str());
		else  STR = "";

		if (!bv.is_type(chaiscript::user_type<void>())) {
			try {
				auto str = engine->call_function(engine->eval("to_string"), bv);
				if (str.is_type(chaiscript::user_type<std::string>())) {
					STR = chaiscript::boxed_cast<std::string>(str).c_str();
				}
				else if (str.is_type(chaiscript::user_type<cweeStr>())) {
					STR = chaiscript::boxed_cast<cweeStr>(str);
				}
			}
			catch (...) {}
		}
	}
	catch (chaiscript::Boxed_Value e) {
		if (!e.is_type(chaiscript::user_type<void>())) {
			if (e.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(e).pretty_print().c_str();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::dispatch_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::dispatch_error>(e).pretty_print().c_str();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::arity_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::arity_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<chaiscript::exception::arithmetic_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::arithmetic_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<std::runtime_error>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::runtime_error>(e).what();
			}
			else if (e.is_type(chaiscript::user_type<std::exception>())) {
				STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::exception>(e).what();
			}
			else {
				try {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
					else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
					}
				}
				catch (...) {
					STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		else {
			STR = "";
			return STR.c_str();
		}
	}
	catch (chaiscript::exception::eval_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::dispatch_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::arity_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arithmetic_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::runtime_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::exception e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (...) {}
	return STR.c_str();
};
std::vector<std::string> ScriptEngine::DoScript_Cast_VectorStrings(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	std::vector<std::string> result;
	try {
		auto bv = engine->eval(command.c_str());
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		} else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				} 
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {
					
				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<cweeList<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<cweeList<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				} 
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {
					
				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<std::string>>())) {
			result = chaiscript::boxed_cast<std::vector<std::string>>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<cweeStr>>())) {
			auto res = chaiscript::boxed_cast<std::vector<cweeStr>>(bv);
			for (auto& x : res) {
				result.push_back(x.c_str());
			}
			return result;
		}
		else {
			// do what we can... 




		}
	}
	catch (chaiscript::Boxed_Value bv) {
		if (bv.is_type(chaiscript::user_type<void>())) {
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<chaiscript::Boxed_Value>>())) {
			auto res = chaiscript::boxed_cast<std::vector<chaiscript::Boxed_Value>>(bv);
			for (auto& x : res) {
				if (x.is_type(chaiscript::user_type<std::string>())) {
					result.push_back(chaiscript::boxed_cast<std::string>(x));
				}
				else if (x.is_type(chaiscript::user_type<cweeStr>())) {
					result.push_back(chaiscript::boxed_cast<cweeStr>(x).c_str());
				}
				else {

				}
			}
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<std::string>>())) {
			result = chaiscript::boxed_cast<std::vector<std::string>>(bv);
			return result;
		}
		else if (bv.is_type(chaiscript::user_type<std::vector<cweeStr>>())) {
			auto res = chaiscript::boxed_cast<std::vector<cweeStr>>(bv);
			for (auto& x : res) {
				result.push_back(x.c_str());
			}
			return result;
		}
	}
	catch (...) {}
	return result;
};
std::vector< ScriptingNode > ScriptEngine::PreParseScript(std::string command) {
	std::vector< ScriptingNode > out;
	cweeStr STR = "";
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });
	try {
		try {
			AUTO parserFunc = engine->eval("`DoParse`");

			AUTO bv_of_list_of_bv = engine->call_function(parserFunc, chaiscript::var(cweeStr(command.c_str())), chaiscript::var(cweeStr("WaterWatchParse")));

			try {
				AUTO list_of_bv = engine->boxed_cast<std::vector<chaiscript::Boxed_Value>*>(bv_of_list_of_bv);
				if (list_of_bv) {
					out.reserve(list_of_bv->size() + 1);
					for (auto& x : *list_of_bv) {
						try {
							AUTO containerPTR = engine->boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(x);
							if (containerPTR) {
								if (containerPTR->find("node") != containerPTR->end()) {
									try {
										AUTO nodePtr = engine->boxed_cast<chaiscript::AST_Node*>(containerPTR->operator[]("node"));
										if (nodePtr) { 											
											if (nodePtr->filename() != "WaterWatchParse") {
												// this is likely a case of from-scratch evaluation within an evaluation. (i.e. "10 == ${ 10.0 }";
												// to fix this would require upating the "ListNodes" script function to "fix" the column and line notices based on the parenthood and/or based on the filename of the parser.
												continue;
											}
											bool WrongNamespace = false;
											for (auto& child : nodePtr->get_children()) {
												if (child.get().filename() != "WaterWatchParse") {
													WrongNamespace = true;
													break;
												}
											}
											if (WrongNamespace) continue;

											auto temp = ScriptingNode();
											if (containerPTR->find("type") != containerPTR->end()) {
												try {
													std::string* type_str_ptr = engine->boxed_cast<std::string*>(containerPTR->operator[]("type"));
													if (type_str_ptr) {
														temp.typeHint = *type_str_ptr;
													}
													else {
														temp.typeHint = "";
													}
												}
												catch (...) { temp.typeHint = ""; }
											}
											else {
												temp.typeHint = "";
											}

											if (containerPTR->find("depth") != containerPTR->end()) {
												try {
													int* depth_ptr = engine->boxed_cast<int*>(containerPTR->operator[]("depth"));
													if (depth_ptr) {
														temp.depth = *depth_ptr;
													}
													else {
														temp.depth = 0;
													}
												}
												catch (...) { temp.depth = 0; }
											}
											else {
												temp.depth = 0;
											}

											temp.text = nodePtr->text.c_str();
											temp.startLine = nodePtr->start().line - 1;
											temp.startColumn = nodePtr->start().column;
											temp.endLine = nodePtr->end().line - 1;
											temp.endColumn = nodePtr->end().column;
											temp.type = static_cast<WaterWatchEnums::ScriptNodeType>(static_cast<int>(nodePtr->identifier));

											out.push_back(temp);
											
										}
										else {
										}
									} catch (...) {}
								}
								else {
								}
							}
							else {
							}
						} catch (...) {}
					}
				}
				return out;
			}
			catch (chaiscript::Boxed_Value e) {
				if (!e.is_type(chaiscript::user_type<void>())) {
					try {
						STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
						auto str = engine->call_function(engine->eval("to_string"), e);
						if (str.is_type(chaiscript::user_type<std::string>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
						}
						else if (str.is_type(chaiscript::user_type<cweeStr>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
						}
						else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
							STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
						}
					}
					catch (chaiscript::exception::eval_error e) {
						STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
					}
					catch (chaiscript::exception::dispatch_error e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (chaiscript::exception::arity_error e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (chaiscript::exception::arithmetic_error e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (std::runtime_error e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (std::exception e) {
						STR = (cweeStr("Error: ") + cweeStr(e.what()));
					}
					catch (...) {
						STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
					}
				}
			}
			catch (chaiscript::exception::eval_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
			}
			catch (chaiscript::exception::dispatch_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arity_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arithmetic_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::runtime_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::exception e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (...) {
				STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 1"));
			}
			if (STR != "") { 
				ScriptingNode node; 
				node.depth = 0; 
				node.type = WaterWatchEnums::ScriptNodeType::Error; 
				node.text = STR; 
				auto splitter = STR.Split(" at (");
				if (splitter.getNumVars() >= 2) {
					auto splitter2 = splitter[1].Split(")");
					if (splitter2.getNumVars() >= 1) {
						auto line_col = splitter2[0].Split(",");
						for (auto& c : line_col) c.Replace(" ", "");
						if (line_col.getNumVars() >= 2) {
							node.startLine = 0;
							node.endLine = (int)(line_col[0]);
							node.startColumn = 0;
							node.endColumn = (int)(line_col[1]);
						}
					}
				}
				out.push_back(node); 
				STR = ""; 
			}

			return out;
		}
		catch (chaiscript::Boxed_Value e) {
			if (!e.is_type(chaiscript::user_type<void>())) {
				try {
					STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
					auto str = engine->call_function(engine->eval("to_string"), e);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
					}
					else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
						STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
					}
				}
				catch (chaiscript::exception::eval_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
				}
				catch (chaiscript::exception::dispatch_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arity_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (chaiscript::exception::arithmetic_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::runtime_error e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (std::exception e) {
					STR = (cweeStr("Error: ") + cweeStr(e.what()));
				}
				catch (...) {
					STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
				}
			}
		}
		catch (chaiscript::exception::eval_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
		}
		catch (chaiscript::exception::dispatch_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arity_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (chaiscript::exception::arithmetic_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::runtime_error e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (std::exception e) {
			STR = (cweeStr("Error: ") + cweeStr(e.what()));
		}
		catch (...) {
			STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 2"));
		}
		if (STR != "") {
			ScriptingNode node;
			node.depth = 0;
			node.type = WaterWatchEnums::ScriptNodeType::Error;
			node.text = STR;
			auto splitter = STR.Split(" at (");
			if (splitter.getNumVars() >= 2) {
				auto splitter2 = splitter[1].Split(")");
				if (splitter2.getNumVars() >= 1) {
					auto line_col = splitter2[0].Split(",");
					for (auto& c : line_col) c.Replace(" ", "");
					if (line_col.getNumVars() >= 2) {
						node.startLine = 0;
						node.endLine = (int)(line_col[0]);
						node.startColumn = 0;
						node.endColumn = (int)(line_col[1]);
					}
				}
			}
			out.push_back(node);
			STR = "";
		}
	}
	catch (chaiscript::Boxed_Value e) {
		if (!e.is_type(chaiscript::user_type<void>())) {
			try {
				STR = cweeStr::printf("Retrieved Thrown Object of Type: '%s'.", e.get_type_info().name());
				auto str = engine->call_function(engine->eval("to_string"), e);
				if (str.is_type(chaiscript::user_type<std::string>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<std::string>(str).c_str();
				}
				else if (str.is_type(chaiscript::user_type<cweeStr>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<cweeStr>(str);
				}
				else if (str.is_type(chaiscript::user_type<chaiscript::exception::eval_error>())) {
					STR = cweeStr("Error: ") + chaiscript::boxed_cast<chaiscript::exception::eval_error>(str).pretty_print().c_str();
				}
			}
			catch (chaiscript::exception::eval_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
			}
			catch (chaiscript::exception::dispatch_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arity_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (chaiscript::exception::arithmetic_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::runtime_error e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (std::exception e) {
				STR = (cweeStr("Error: ") + cweeStr(e.what()));
			}
			catch (...) {
				STR = cweeStr::printf("Error: Could Not Parse Thrown Object of Type: '%s'.", e.get_type_info().name());
			}
		}
	}
	catch (chaiscript::exception::eval_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.pretty_print()));
	}
	catch (chaiscript::exception::dispatch_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arity_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (chaiscript::exception::arithmetic_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::runtime_error e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (std::exception e) {
		STR = (cweeStr("Error: ") + cweeStr(e.what()));
	}
	catch (...) {
		STR = (cweeStr("Error: ") + cweeStr("UNKNOWN ERR 3"));
	}
	if (STR != "") {
		ScriptingNode node;
		node.depth = 0;
		node.type = WaterWatchEnums::ScriptNodeType::Error;
		node.text = STR;
		auto splitter = STR.Split(" at (");
		if (splitter.getNumVars() >= 2) {
			auto splitter2 = splitter[1].Split(")");
			if (splitter2.getNumVars() >= 1) {
				auto line_col = splitter2[0].Split(",");
				for (auto& c : line_col) c.Replace(" ", "");
				if (line_col.getNumVars() >= 2) {
					node.startLine = 0;
					node.endLine = (int)(line_col[0]);
					node.startColumn = 0;
					node.endColumn = (int)(line_col[1]);
				}
			}
		}
		out.push_back(node);
		STR = "";
	}

	return out;
};
std::vector<std::string> ScriptEngine::CompatibleFunctions(std::string TypeName) {
	return DoScript_Cast_VectorStrings(cweeStr::printf("return `%s`.get_compatible_functions().keys();", TypeName.c_str()).c_str());
};
std::vector<std::string> ScriptEngine::FunctionsThatStartWith(std::string startsWith) {
	return DoScript_Cast_VectorStrings(cweeStr::printf("get_functions_that_start_with(\"%s\")", startsWith.c_str()).c_str());
};
void ScriptEngine::StopCurrentScript() {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });
	if (engine) {
		engine->get_eval_engine().CancelCurrentScript();
	}
};


std::string ScriptEngine::Cast_String(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	std::string out;
	AUTO val = chaiscript::boxed_cast<std::string*>(bv);
	if (val) {
		out = *val;
	}
	return out;
};
Color_Interop ScriptEngine::Cast_Color(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	Color_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_Color*>(bv);
	if (val) {
		out.R = val->R;
		out.G = val->G;
		out.B = val->B;
		out.A = val->A;
	}
	return out;
};
MapIcon_Interop ScriptEngine::Cast_MapIcon(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	MapIcon_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapIcon*>(bv);
	if (val) {
		out.color.R = val->color.R;
		out.color.G = val->color.G;
		out.color.B = val->color.B;
		out.color.A = val->color.A;
		out.size = val->size;
		out.longitude = val->longitude;
		out.latitude = val->latitude;
		out.HideOnCollision = val->HideOnCollision;
		out.IconPathGeometry = val->IconPathGeometry.c_str();
		out.Label = val->Label.c_str();
	}
	return out;
};
MapPolyline_Interop ScriptEngine::Cast_MapPolyline(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO bv = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	MapPolyline_Interop out;
	AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapPolyline*>(bv);
	if (val) {
		out.color.R = val->color.R;
		out.color.G = val->color.G;
		out.color.B = val->color.B;
		out.color.A = val->color.A;
		out.thickness = val->thickness;
		out.dashed = val->dashed;

		out.coordinates.reserve(val->coordinates.size() + 1);
		for (auto& x : val->coordinates) { out.coordinates.push_back(Pair<double, double>(x.first, x.second)); }
	}
	return out;
};
MapLayer_Interop ScriptEngine::Cast_MapLayer(std::string command) {
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine(ptr, [](void* p) { return static_cast<chaiscript::WaterWatch_ChaiScript*>(p); });

	AUTO boxed = engine->eval(command.c_str(), chaiscript::Exception_Handler());
	MapLayer_Interop out2;
	AUTO val2 = chaiscript::boxed_cast<chaiscript::UI_MapLayer*>(boxed);
	if (val2) {
		for (int i = 0; i < val2->Children.size(); i++) {
			auto& bv = val2->Children[i];
			if (bv.is_type(chaiscript::user_type<chaiscript::UI_MapIcon>())) {
				AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapIcon*>(bv);
				if (val) {
					MapIcon_Interop& out = out2.icons[i];
					out.color.R = val->color.R;
					out.color.G = val->color.G;
					out.color.B = val->color.B;
					out.color.A = val->color.A;
					out.size = val->size;
					out.longitude = val->longitude;
					out.latitude = val->latitude;
					out.HideOnCollision = val->HideOnCollision;
					out.IconPathGeometry = val->IconPathGeometry.c_str();
					out.Label = val->Label.c_str();
				}
			}
			else if (bv.is_type(chaiscript::user_type<chaiscript::UI_MapPolyline>())) {
				AUTO val = chaiscript::boxed_cast<chaiscript::UI_MapPolyline*>(bv);
				if (val) {
					MapPolyline_Interop& out = out2.polylines[i];
					out.color.R = val->color.R;
					out.color.G = val->color.G;
					out.color.B = val->color.B;
					out.color.A = val->color.A;
					out.thickness = val->thickness;
					out.dashed = val->dashed;
					out.coordinates.reserve(val->coordinates.size() + 1);
					for (auto& x : val->coordinates) { out.coordinates.push_back(Pair<double, double>(x.first, x.second)); }
				}
			}
		}
	}
	return out2;
};
#pragma endregion

#pragma region WATERWATCH
void WaterWatch::SubmitToast(std::string title, std::string content) {
	cweeToasts->submitToast(title.c_str(), content.c_str());
};
Pair<bool, Pair<std::string, std::string>> WaterWatch::TryGetToast() {
	static auto empty = Pair<bool, Pair<std::string, std::string>>(false, Pair<std::string, std::string>("", ""));
	cweeStr title, desc;

	if (cweeToasts->tryGetToast(title, desc))
		return Pair<bool, Pair<std::string, std::string>>(true, Pair<std::string, std::string>(std::string(title.c_str()), std::string(desc.c_str())));
	else 
		return empty;
};
Pair<int, Pair<std::string, std::vector<std::string>>> WaterWatch::TryGetAppRequest() {
	static auto empty = Pair<int, Pair<std::string, std::vector<std::string>>>(-1, Pair<std::string, std::vector<std::string>>("", std::vector<std::string>()));

	std::pair<
		int, // ID
		cweeUnion<
		cweeStr, // Function Name
		cweeThreadedList<cweeStr>, // Arguments (optional)
		cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
		std::vector<std::string> params; for (auto& p : request.second.get<1>()) params.push_back(p.c_str());
		return Pair<int, Pair<std::string, std::vector<std::string>>>(request.first, Pair<std::string, std::vector<std::string>>(request.second.get<0>().c_str(), params));
	}
	else {
		return empty;
	}
};
void WaterWatch::CompleteAppRequest(int ID, std::string reply) {
	AppLayerRequests->FinishRequest(ID, reply.c_str());
};

std::string WaterWatch::GetDataDirectory() {
	return fileSystem->getDataFolder().c_str(); 
};
void WaterWatch::SetDataDirectory(std::string dir) { 	
	fileSystem->setDataFolder(dir.c_str()); 
};
Pair<double, double> WaterWatch::GeocodeAddress(std::string address) {
	auto t = geocoding->GetLongLat(address.c_str());
	return std::make_pair(t.x, t.y);
};
std::string WaterWatch::GeocodeAddress(double longitude, double latitude) {
	return geocoding->GetAddress(vec2d(longitude, latitude)).c_str();
};
double WaterWatch::GeocodeElevation(double longitude, double latitude) {
	return geocoding->GetElevation(vec2d(longitude, latitude));
};
Pair<double, double> WaterWatch::ValidateCoordinates(double longitude, double latitude) {
	if (longitude >= -180 && longitude <= 180 && latitude >= -90 && latitude <= 90) {
		return Pair<double, double>(longitude, latitude);
	}
	else {
		AUTO coords = geocoding->GetLongLat(longitude, latitude);
		return Pair<double, double>(coords.x, coords.y);
	}
};
double WaterWatch::Encode_2D_to_1D(double x, double y, double H) {
	return cweeInterpolatedMatrix<float>::Encode_2D_to_1D(x, y, H);
};
Pair<double, double> WaterWatch::Decode_1D_to_2D(double l, double H) {
	auto x = cweeInterpolatedMatrix<float>::Decode_1D_to_2D(l, H);
	return std::make_pair((double)x.first, (double)x.second);
};

std::vector<float> WaterWatch::PredictNext(std::vector<double> const& data, int nextN) {
	
	cweeList<float> labels = data;
	cweeList<cweeList<float>> features; {
		features.Append(data);
		features.Append(data);
	}

	std::pair<vec2, vec2> fit;

	AUTO ml_result = cweeMachineLearning::Learn(labels, features, &fit);
	return { ml_result.performance.x, ml_result.performance.y, ml_result.performance.z, ml_result.performance.w };
};
cweeDateTime WaterWatch::getCurrentTime() {
	return cweeDateTime((u64)fileSystem->getCurrentTime());
};

int WaterWatch::RandomInt(int min, int max) { return cweeRandomFloat(min,max); };
float WaterWatch::RandomFloat(float min, float max) { return cweeRandomFloat(min, max); };
std::string WaterWatch::GetBestMatch(std::string const& input, std::vector<std::string> const& options) {
	cweeStr a = input.c_str();
	cweeThreadedList<cweeStr> op(options.size() + 16);

	for (auto& x : options) op.Append(x.c_str());

	cweeStr bestMatch = a.BestMatch(op);
	return bestMatch.c_str();
};

int WaterWatch::GetNumMultithreadingCores() {
	return WindowsPlatform::GetCPUInfo().logicalProcessorCount;
};
int WaterWatch::GetNumLogicalCoresOnMachine() {
	return WindowsPlatform::GetCPUInfo().logicalProcessorCount;
};
int WaterWatch::GetNumPhysicalCoresOnMachine() {
	return WindowsPlatform::GetCPUInfo().processorCoreCount;
};
float WaterWatch::GetPercentMemoryUsedOfMachine() {
	return Computer_Usage().PercentMemoryUsed();
};
float WaterWatch::GetPercentCpuUsedOfMachine() {
	return WindowsPlatform::GetCPULoad();
};

void WaterWatch::AddToLog(std::string filePath, std::string content) {
	cweeStr previousFile;
	fileSystem->readFileAsCweeStr(previousFile, filePath.c_str());
	fileSystem->writeFileFromCweeStr(filePath.c_str(), previousFile.AddToDelimiter(content.c_str(), "\n"));
};
double WaterWatch::GetNanosecondsSinceStart() {
	return clock_ns();
};

Awaiter WaterWatch::DoScript(std::string command) {
	Awaiter toReturn;

	cweeJob task1([](std::string const& cmd)-> std::string {
		static cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine = cweeSharedPtr<chaiscript::WaterWatch_ChaiScript>(new chaiscript::WaterWatch_ChaiScript(), [](chaiscript::WaterWatch_ChaiScript* p) {
			delete p;
		});
		try {
			auto bv = engine->eval(cweeStr(cmd.c_str()).c_str());
			if (!bv.is_type(chaiscript::user_type<void>())) {
				try {
					auto str = engine->call_function(engine->eval("to_string"), bv);
					if (str.is_type(chaiscript::user_type<std::string>())) {
						std::string STR = chaiscript::boxed_cast<std::string>(str);
						return STR.c_str();
					}
					else if (str.is_type(chaiscript::user_type<cweeStr>())) {
						cweeStr STR = chaiscript::boxed_cast<cweeStr>(str);
						return STR.c_str();
					}
				}
				catch (...) {}
			}
		}
		catch (std::runtime_error e) {
			return (cweeStr("Error: ") + cweeStr(e.what())).c_str();
		}
		catch (...) {}

		return "";
		}, command);
	task1.ContinueWith([task1](Awaiter& place_results) ->void {
		*place_results.data_m = task1.GetResult().cast<std::string>();
		*place_results.isFinished_m = true;
		}, toReturn);
	task1.AsyncInvoke();

	return toReturn;
};

/*
int WaterWatch::Data_CreatePattern();
void WaterWatch::Data_DeletePattern(int index);
void WaterWatch::Data_ClearPattern(int index);
void WaterWatch::Data_AppendData(int index, double time, float value);
float WaterWatch::Data_GetValue(int index, double time);
int WaterWatch::Data_CreateMatrix();
void WaterWatch::Data_DeleteMatrix(int index);
void WaterWatch::Data_ClearMatrix(int index);
void WaterWatch::Data_AppendData(int index, double column, double row, float value);
float WaterWatch::Data_GetValue(int index, double column, double row);
std::vector<float> WaterWatch::Data_GetMatrix(int index, double left, double top, double right, double bottom, int nCol, int nRow);
int WaterWatch::Data_CreateSpatialAsset();
void WaterWatch::Data_DeleteSpatialAsset(int index);
void WaterWatch::Data_ClearSpatialAsset(int index);
void WaterWatch::Data_AppendData(int index, std::string const& name, std::string const& value);
void WaterWatch::Data_AppendData(int index, std::string const& name, cweeDateTime const& time, std::string const& value);
void WaterWatch::Data_AppendData(int index, std::string const& name, cweeDateTime const& time, float value);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index, double from, double till);
std::vector<Pair<cweeDateTime, float>>	WaterWatch::Data_GetPattern(int index, double from, double till, double step);
*/



#pragma endregion

