#pragma hdrstop
#include "precompiled.h"

float	cweeVecX::temp[VECX_MAX_TEMP + 4];
float*	cweeVecX::tempPtr = (float*)(((int)cweeVecX::temp + 15) & ~15);
int		cweeVecX::tempIndex = 0;