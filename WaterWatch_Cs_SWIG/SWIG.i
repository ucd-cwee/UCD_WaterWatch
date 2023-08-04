/* WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code. */

%module Conv

%{
#include "../WaterWatch_Cs_SWIG/Header.h"
%}

%include "stl.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_shared_ptr.i"
%include "../WaterWatch_Cs_SWIG/Header.h"

/* define template instantiations */
%template(vector_string) std::vector<std::string>;
%template(pair_timeseries) Pair<cweeDateTime, float>;
%template(pair_string_vector_string) Pair<std::string, std::vector<std::string>>;
%template(pair_int_pair_string_vector_string) Pair<int, Pair<std::string, std::vector<std::string>>>;
%template(pair_double_double) Pair<double, double>;
%template(pair_string_string) Pair<std::string, std::string>;
%template(pair_bool_pair_string_string) Pair<bool, Pair<std::string, std::string>>;
%template(vector_float) std::vector<float>;
%template(vector_double) std::vector<double>;
%template(vector_pair_timeseries) std::vector<Pair<cweeDateTime, float>>;
%template(vector_pair_double_double) std::vector<Pair<double, double>>;
%template(vector_scriptingnode) std::vector<ScriptingNode>;
%template(vector_background) std::vector<MapBackground_Interop>;
%template(vector_colors) std::vector<Color_Interop>;
%template(map_int_background) std::map<int, MapBackground_Interop>;
%template(map_int_polyline) std::map<int, MapPolyline_Interop>;
%template(map_int_icon) std::map<int, MapIcon_Interop>;
