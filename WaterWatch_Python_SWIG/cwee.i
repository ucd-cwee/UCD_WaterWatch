/* WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code. */

%module cwee
%{
#include "cwee.h"
%}

/* INCLUDE THE SWIG BUILT-IN PYTHON CONVERSION LIBRARIES */
%include stl.i
/* %include std_pair.i
%include std_map.i */

namespace std {
    %template(StringVector)  vector<string>;
    %template(IntVector)     vector<int>;
    %template(FloatVector)   vector<float>;
    %template(DoubleVector)  vector<double>;

    %template(StringPair) pair<std::string, std::string>;
    %template(StringMap) map<std::string, std::string>;
}

/* ADD COMMENTS FOR FUNCTIONS ( MUST BE BEFORE THE HEADER FILE THAT DECLARES THE FUNCTIONS ) */
%feature("docstring") WaterWatch::TestVector "Generates random numbers"
%feature("docstring") WaterWatch::TestVector2 "Returns the size of the input vector"
%feature("docstring") WaterWatch "Static class (i.e. namespace) that contains the main functions of the library"

/* DECLARE THE FUNCTIONS */
%include "cwee.h"

/* DECLARE THE REMAINING TEMPLATE TYPES */
%template(pair_timeseries) Pair<cweeDateTime, float>;
%template(pair_double_double) Pair<double, double>;
%template(pair_string_string) Pair<std::string, std::string>;
%template(pair_bool_pair_string_string) Pair<bool, Pair<std::string, std::string>>;
%template(pair_string_vector_string) Pair<std::string, std::vector<std::string>>;
%template(pair_int_pair_string_vector_string) Pair<int, Pair<std::string, std::vector<std::string>>>;
%template(vector_pair_timeseries) std::vector<Pair<cweeDateTime, float>>;
%template(vector_pair_double_double) std::vector<Pair<double, double>>;
%template(vector_scriptingnode) std::vector<ScriptingNode>;

%template() std::pair<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;
%template(pymap) std::map<swig::SwigPtr_PyObject, swig::SwigPtr_PyObject>;