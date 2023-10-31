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
#include "AppLayerRequests.h"

/* for the backend layer to insert to, with a job to await */
std::pair<int, cweeJob> appLayerRequests::AddRequest(method_type const& methodName, arguments_type const& arguments) {
	int n = jobNums.Increment();
	AUTO result = container_type(methodName, arguments, cweeSharedPtr<cweeStr>(cweeStr("")));
	requests.Emplace(n, result);

	cweeJob out([this](int num, cweeStr& get) -> cweeStr { return get; }, n, result.get<2>());

	jobs.Emplace(n, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>>(out, result.get<2>()));

	return std::pair<int, cweeJob>(n, out);
};
/* for the app layer to dequeue from */
std::pair<int, appLayerRequests::container_type> appLayerRequests::DequeueRequest() {
	AUTO g = requests.Guard();
	AUTO pair = requests.unsafe_pair_at_index(0);
	if (pair.first >= 0) {
		requests.UnsafeErase(pair.first);
		return std::pair<int, container_type>(pair.first, *pair.second);
	}
	return std::pair<int, container_type>(-1, container_type());
};
/* for the app layer to check that there's work to be done */
int	appLayerRequests::Num() {
	return requests.Num();
};
// for the app layer to respond to
void appLayerRequests::FinishRequest(int index, cweeStr const& reply) {
	{
		auto ptr = jobs.at(index);
		if (ptr) {
			{
				ptr->get<1>().Lock();
				ptr->get<1>().UnsafeGet()->operator=(reply);
				ptr->get<1>().Unlock();
			}
			{
				ptr->get<0>().AsyncInvoke();
			}
			{
				jobs.Erase(index);
			}
		}
	}
};
// for the backend layer to hard - wait for the app layer to perform the requested job
cweeStr appLayerRequests::Query(method_type const& methodName, arguments_type const& arguments) {
	cweeStr out;

	AUTO req = AddRequest(methodName, arguments);
	cweeAny reply = req.second.Await();
	if (reply) {
		out = reply.cast<cweeStr>();
	}

	return out;
};
cweeSharedPtr<appLayerRequests> AppLayerRequests = make_cwee_shared<appLayerRequests>();