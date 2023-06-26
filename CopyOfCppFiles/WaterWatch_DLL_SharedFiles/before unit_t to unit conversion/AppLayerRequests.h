#pragma once
#include "Precompiled.h"

/*! Class that allows backend to push requests or jobs forwards to the UI or user-layer to process asynchronously. */
class appLayerRequests {
public:
	using arguments_type = typename cweeThreadedList<cweeStr>;
	using method_type = typename cweeStr;
	using container_type = typename cweeUnion<cweeStr, cweeThreadedList<cweeStr>, cweeSharedPtr<cweeStr>>;

private:
	cweeThreadedMap< int, container_type >									requests;
	cweeThreadedMap< int, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>> >		jobs;
	cweeSysInterlockedInteger												jobNums;

public:
	/* for the backend layer to insert to, with a job to await */
	std::pair<int, cweeJob> AddRequest(method_type const& methodName, arguments_type const& arguments = arguments_type()) {
		int n = jobNums.Increment();
		AUTO result = container_type(methodName, arguments, cweeSharedPtr<cweeStr>(cweeStr("")));
		requests.Emplace(n, result);

		cweeJob out([this](int num, cweeStr& get) -> cweeStr { return get; }, n, result.get<2>());

		jobs.Emplace(n, cweeUnion<cweeJob, cweeSharedPtr<cweeStr>>(out, result.get<2>()));

		return std::pair<int, cweeJob>(n, out);
	};
	/* for the app layer to dequeue from */
	std::pair<int, container_type> DequeueRequest() {
		std::pair<int, container_type> out = std::pair<int, container_type>(-1, container_type());
		if (1) {
			AUTO g = requests.Guard();
			AUTO pair = requests.unsafe_pair_at_index(0);
			if (pair.first >= 0) {
				out.first = pair.first;
				out.second = *pair.second;
				requests.UnsafeErase(pair.first);
			}
		}
		return out;
	};
	/* for the app layer to check that there's work to be done */
	int	Num() {
		return requests.Num();
	};
	// for the app layer to respond to
	void FinishRequest(int index, cweeStr const& reply) {
		//AUTO g = jobs.Guard();
		//if (g)
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
	cweeStr Query(method_type const& methodName, arguments_type const& arguments = arguments_type()) {
		cweeStr out;

		AUTO req = AddRequest(methodName, arguments);
		cweeAny reply = req.second.Await();
		if (reply) {
			out = reply.cast<cweeStr>();
		}

		return out;
	};
};
static cweeSharedPtr<appLayerRequests> AppLayerRequests = make_cwee_shared<appLayerRequests>();