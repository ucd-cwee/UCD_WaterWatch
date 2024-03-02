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
#include "cweeThreadedMap.h"
#include "SharedPtr.h"
#include "InterlockedValues.h"
#include "Strings.h"
#include "List.h"
#include "cweeJob.h"
#include "DelayedInstantiation.h"
#include "../FiberTasks/Fibers.h"

/*! Class that allows backend to push requests or jobs forwards to the UI or user-layer to process asynchronously. */
class appLayerRequests {
public:
	using arguments_type = cweeThreadedList<cweeStr>;
	using method_type = cweeStr;
	using container_type = cweeUnion<cweeStr, cweeThreadedList<cweeStr>, cweeSharedPtr<cweeStr>>;

	appLayerRequests() : requests(), jobs(), jobNums(0) {};

private:
	fibers::containers::queue<std::pair<int, container_type>>               requests;
	cweeThreadedMap< int, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>> >		jobs;
	cweeSysInterlockedInteger												jobNums;

public:
	/* for the backend layer to insert to, with a job to await */
	std::pair<int, cweeJob> AddRequest(method_type const& methodName, arguments_type const& arguments = arguments_type());
	/* for the app layer to dequeue from */
	std::pair<int, container_type> DequeueRequest();
	/* for the app layer to check that there's work to be done */
	int	Num();
	// for the app layer to respond to
	void FinishRequest(int index, cweeStr const& reply);
	// for the backend layer to hard - wait for the app layer to perform the requested job
	cweeStr Query(method_type const& methodName, arguments_type const& arguments = arguments_type());
};
extern DelayedInstantiation< appLayerRequests > AppLayerRequests;

// extern appLayerRequests* AppLayerRequests;
// extern cweeSharedPtr<appLayerRequests> AppLayerRequests;