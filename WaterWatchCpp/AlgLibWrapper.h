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
#include "Precompiled.h"
#include "List.h"

#include "AlgLib/alglibinternal.h"
#include "AlgLib/alglibmisc.h"
#include "AlgLib/ap.h"
#include "AlgLib/dataanalysis.h"
#include "AlgLib/diffequations.h"
#include "AlgLib/fasttransforms.h"
#include "AlgLib/integration.h"
#include "AlgLib/interpolation.h"
#include "AlgLib/kernels_avx2.h"
#include "AlgLib/kernels_fma.h"
#include "AlgLib/kernels_sse2.h"
#include "AlgLib/linalg.h"
#include "AlgLib/optimization.h"
#include "AlgLib/solvers.h"
#include "AlgLib/specialfunctions.h"
#include "AlgLib/statistics.h"

namespace alglibwrapper {
	// using namespace alglib;

    class Interpolator {
    public:
        static double SparseMatrixInterp(cweeThreadedList<cweeUnion<double, double, double>> const& data, double X, double Y) {
            double out = 0; 
            if (data.Num() > 0) {
                {
                    alglib::real_2d_array arr;
                    arr.attach_to_ptr(data.Num(), 3, (double*)(void*)data.Ptr()); // ptr);
                    {
                        alglib::rbfmodel model;
                        alglib::rbfcreate(2, 1, model);
                        rbfsetpoints(model, arr);
                        alglib::rbfreport rep;
                        alglib::rbfsetalgohierarchical(model, 1.0, 3, 0.0);
                        alglib::rbfbuildmodel(model, rep);
                        out = alglib::rbfcalc2(model, X, Y);
                    }
                }
            }
            return out;
        };
        static cweeThreadedList<double> PredictNextInSequence(cweeThreadedList<double> const& sequence, int N_to_Predict) {
            using namespace alglib;

            cweeThreadedList<double> out;

            // Here we demonstrate SSA forecasting on some toy problem with clearly visible linear trend and small amount of noise.
            ssamodel s;
            real_1d_array x;
            x.attach_to_ptr(sequence.Num(), const_cast<double*>(sequence.Ptr()));

            // First, we create SSA model, set its properties and add dataset.            
            // We use window with width=3 and configure model to use direct SSA
            // algorithm - one which runs exact O(N*W^2) analysis - to extract
            // two top singular vectors. Well, it is toy problem :)            
            // NOTE: SSA model may store and analyze more than one sequence
            //       (say, different sequences may correspond to data collected
            //       from different devices)
            ssacreate(s);
            ssasetwindow(s, 3);
            ssaaddsequence(s, x);
            ssasetalgotopkdirect(s, 2);

            // Now we begin analysis. Internally SSA model stores everything it needs:
            // data, settings, solvers and so on. Right after first call to analysis-
            // related function it will analyze dataset, build basis and perform analysis.            
            // Subsequent calls to analysis functions will reuse previously computed
            // basis, unless you invalidate it by changing model settings (or dataset).            
            // In this example we show how to use ssaforecastlast() function, which
            // predicts changed in the last sequence of the dataset. If you want to
            // perform prediction for some other sequence, use ssaforecastsequence().
            real_1d_array trend;
            ssaforecastlast(s, N_to_Predict, trend);

            for (size_t i = 0; i < trend.length(); i++) { out.Append(trend[i]); }

            return out;
        };

    };

};