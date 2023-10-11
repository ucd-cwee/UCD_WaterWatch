%module foo
%{
#include "foo.h"
%}


%include "stl.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_shared_ptr.i"
%include "foo.h"
