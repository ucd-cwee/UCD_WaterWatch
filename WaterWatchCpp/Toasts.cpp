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
#include "Toasts.h"
#include "LinkedList.h"

class cweeToastsLocal final : public detail::cweeToasts_t {
private:
	cweeSharedPtr<cweeLinkedList<cweeUnion<cweeStr, cweeStr>>> Toasts;

public:
	cweeToastsLocal() : detail::cweeToasts_t(), Toasts(make_cwee_shared<cweeLinkedList<cweeUnion<cweeStr, cweeStr>>>()) {};
	void submitToast(cweeStr const& title, cweeStr const& content) const {
		AUTO g = Toasts.Guard();
		Toasts.UnsafeGet()->Append(cweeUnion<cweeStr, cweeStr>(title, content));
	};
	bool tryGetToast(cweeStr& title, cweeStr& content) const {
		AUTO g = Toasts.Guard();
		AUTO p = Toasts.UnsafeGet();
		if (p && p->Num() > 0) {
			AUTO toast = p->operator[](p->Num() - 1);
			title = toast.get<0>();
			content = toast.get<1>();
			p->RemoveIndexFast(p->Num() - 1);
			return true;
		}
		return false;
	};
};
cweeSharedPtr<detail::cweeToasts_t> cweeToasts = make_cwee_shared<cweeToastsLocal>(new cweeToastsLocal()).CastReference<detail::cweeToasts_t>();