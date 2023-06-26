#pragma once
#include "Precompiled.h"

static cweeSharedPtr<cweeLinkedList<cweeUnion<cweeStr,cweeStr>>> Toasts = make_cwee_shared<cweeLinkedList<cweeUnion<cweeStr, cweeStr>>>(new cweeLinkedList<cweeUnion<cweeStr, cweeStr>>());
class cweeToasts {
public:
	static void submitToast(cweeStr const& title, cweeStr const& content) {
		AUTO g = Toasts.Guard();
		Toasts.UnsafeGet()->Append(cweeUnion<cweeStr, cweeStr>(title, content));
	};
	static bool tryGetToast(cweeStr& title, cweeStr& content) {
		AUTO g = Toasts.Guard();
		AUTO p = Toasts.UnsafeGet();
		if (p && p->Num() > 0) {
			AUTO toast = p->operator[](p->Num()-1);
			title = toast.get<0>();
			content = toast.get<1>();
			p->RemoveIndexFast(p->Num() - 1);
			return true;
		}
		return false;			
	};
};