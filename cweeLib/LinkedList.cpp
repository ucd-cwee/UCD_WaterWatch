
#ifndef __LINKED_LIST_CPP__
#define __LINKED_LIST_CPP__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"
#pragma endregion

#pragma region "cweeLinkedListNode definitions"

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>& cweeLinkedListNode<T, numChildren>::operator=(const cweeLinkedListNode<T, numChildren>& other) {
	childrenPerNode = other.childrenPerNode;
	parent = other.parent;
	parentIndex = other.parentIndex;
	prev = other.prev;
	next = other.next;
	_relativeIndex = other._relativeIndex;
	_data = other._data;

	return *this;
};

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>& cweeLinkedListNode<T, numChildren>::operator=(cweeLinkedListNode<T, numChildren>&& other) {
	childrenPerNode = other.childrenPerNode;
	parent = other.parent;
	parentIndex = other.parentIndex;
	prev = other.prev;
	next = other.next;
	_relativeIndex = other._relativeIndex;
	_data = other._data;

	other.parent = nullptr;
	other.parentIndex = 0;
	other.prev = nullptr;
	other.next = nullptr;
	other._relativeIndex = 0;
	other._data.Clear();

	return *this;
};

template <typename T, int numChildren>
void cweeLinkedListNode<T, numChildren>::Append() {
	// if we are about to extend beyond our allotment, then we need to split.
	if (childrenPerNode < NumRef() + 1) {
		if (next) {
			// split the node
			Split();

			// add the data to our next neighbor instead, as was originally intended
			{
				next->Append();
			}
		}
		else {
			next = parent->AppendNode();
			next->_relativeIndex = _relativeIndex + NumRef();
			next->prev = this;
			next->Append();
		}
	}
	else {
		// add the data 
		{
			_data.Alloc() = T(); // initialize it, just in case
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
};

template <typename T, int numChildren>
void cweeLinkedListNode<T, numChildren>::Append(const T& obj) {
	// if we are about to extend beyond our allotment, then we need to split.
	if (childrenPerNode < NumRef() + 1) {
		if (next) {
			// split the node
			Split();

			// add the data to our next neighbor instead, as was originally intended
			{
				next->Append(obj);
			}
		}
		else {
			next = parent->AppendNode();
			next->_relativeIndex = _relativeIndex + NumRef();
			next->prev = this;
			next->Append(obj);
		}
	}
	else {
		// add the data 
		{
			_data.Append(obj);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
};

template <typename T, int numChildren>
void cweeLinkedListNode<T, numChildren>::Insert(const T& obj, int relativeIndex) {
	// if we are about to extend beyond our allotment, then we need to split.
	if (relativeIndex < NumRef()) {
		// if needed, split and try again.
		{
			if (childrenPerNode < NumRef() + 1) {
				Split();
				Insert(obj, relativeIndex);
				return;
			}
		}

		// insert it here 
		{
			_data.Insert(obj, relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
	else {
		// insert it downstream
		if (next) {
			next->Insert(obj, relativeIndex - NumRef());
		}
		else {
			Append(obj);
		}
	}
};

template <typename T, int numChildren>
bool cweeLinkedListNode<T, numChildren>::Remove(int relativeIndex) {
	if (relativeIndex < 0) {
		return false;
	}

	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndex(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->Remove(relativeIndex - NumRef());
	}
	return false;
};

template <typename T, int numChildren>
bool cweeLinkedListNode<T, numChildren>::RemoveFast(int relativeIndex) {
	if (relativeIndex < 0) {
		return false;
	}
#if 0
	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndexFast(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#else
	if (relativeIndex < NumRef()) {
		// override the object at "relativeIndex" with the value of the VERY LAST OBJECT in the entire dataset. Instead of finding it by navigating the "next" ptrs, use the parents to teleport there. 
		cweeLinkedListNode<T, numChildren>* lastNode = parent->GetNode(parent->NumNodes() - 1); // worst-case, will return this node
		{
			T& lastObj = lastNode->_data[lastNode->_data.NumRef() - 1];
			_data[relativeIndex] = lastObj;
		}
		// delete the very last object in the entire list
		lastNode->Remove(lastNode->NumRef() - 1); 

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#endif

	return false;
};

template <typename T, int numChildren>
bool cweeLinkedListNode<T, numChildren>::Extract(int relativeIndex, T& out) {
	if (relativeIndex < 0) {
		return false;
	}

	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndex(relativeIndex);
			out = _data[relativeIndex];
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->Extract(relativeIndex - NumRef(), out);
	}
	return false;
};

template <typename T, int numChildren>
bool cweeLinkedListNode<T, numChildren>::ExtractFast(int relativeIndex, T& out) {
	if (relativeIndex < 0) {
		return false;
	}
#if 0
	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndexFast(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#else
	if (relativeIndex < NumRef()) {
		// override the object at "relativeIndex" with the value of the VERY LAST OBJECT in the entire dataset. Instead of finding it by navigating the "next" ptrs, use the parents to teleport there. 
		cweeLinkedListNode<T, numChildren>* lastNode = parent->GetNode(parent->NumNodes() - 1); // worst-case, will return this node
		{
			T& lastObj = lastNode->_data[lastNode->_data.NumRef() - 1];
			out = _data[relativeIndex];
			_data[relativeIndex] = lastObj;
		}
		// delete the very last object in the entire list
		lastNode->Remove(lastNode->NumRef() - 1);

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->ExtractFast(relativeIndex - NumRef(), out);
	}
#endif

	return false;
};

template <typename T, int numChildren>
void cweeLinkedListNode<T, numChildren>::AbsorbTheRight() {
	if (next) {
		cweeLinkedListNode* toDelete = next;

		// move the data
		{
			_data.Append(toDelete->_data);

			//for (T& x : toDelete->_data) {
			//	_data.Append(x);
			//}
			//toDelete->_data.Clear();
		}

		// Change the relationships
		{
			next = toDelete->next;
			if (next) {
				next->prev = this;
			}

			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->parentIndex--;
			}
		}

		// delete the node
		{
			parent->FreeNode(toDelete->parentIndex);
		}
	}
};

template <typename T, int numChildren>
void cweeLinkedListNode<T, numChildren>::Split() {
	if (parent && NumRef() >= 2) {
		// create a node to my right and change assignments 
		{
			if (next) {
				for (cweeLinkedListNode* node = next; node; node = node->next) {
					node->parentIndex++;
				}

				cweeLinkedListNode* newRightNeighbor = parent->InsertNode(parentIndex + 1);

				newRightNeighbor->next = next;
				newRightNeighbor->prev = this;

				next->prev = newRightNeighbor;
				next = newRightNeighbor;
			}
			else {
				cweeLinkedListNode* newRightNeighbor = parent->AppendNode();

				newRightNeighbor->next = nullptr;
				newRightNeighbor->prev = this;

				next = newRightNeighbor;
			}
		}

		// move half of my data to the node to my right
		{
			int start = NumRef() / 2;
			int end = NumRef();
			for (int i = start; i < end; i++) {
				next->_data.Append(_data[i]);
			}
			_data.Resize(_data.NumRef() - (end - start));
		}

		// update the relative index of the node to my right
		{
			next->_relativeIndex = _relativeIndex + NumRef();
		}
	}
};

template <typename T, int numChildren>
T& cweeLinkedListNode<T, numChildren>::absoluteIndex(int absoluteIndex) {
	return _data[absoluteIndex - _relativeIndex];
};

template <typename T, int numChildren>
const T& cweeLinkedListNode<T, numChildren>::absoluteIndex(int absoluteIndex) const {
	return _data[absoluteIndex - _relativeIndex];
};

template <typename T, int numChildren>
T& cweeLinkedListNode<T, numChildren>::relativeIndex(int relativeIndex) {
	return _data[relativeIndex];
};

template <typename T, int numChildren>
const T& cweeLinkedListNode<T, numChildren>::relativeIndex(int relativeIndex) const {
	return _data[relativeIndex];
};

template <typename T, int numChildren>
int cweeLinkedListNode<T, numChildren>::Num() const {
	return _data.Num();
};

template <typename T, int numChildren>
int cweeLinkedListNode<T, numChildren>::NumRef() const {
	return _data.NumRef();
};

template <typename T, int numChildren>
int cweeLinkedListNode<T, numChildren>::MinIndex() const {
	return _relativeIndex;
};

template <typename T, int numChildren>
int cweeLinkedListNode<T, numChildren>::MaxIndex() const {
	return _relativeIndex + NumRef();
};

template <typename T, int numChildren>
cweeThreadedList<T>& cweeLinkedListNode<T, numChildren>::Data() {
	return _data;
};

template <typename T, int numChildren>
const cweeThreadedList<T>& cweeLinkedListNode<T, numChildren>::Data() const {
	return _data;
};

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>* cweeLinkedListNode<T, numChildren>::GetNext() {
	return next;
};

template <typename T, int numChildren>
const cweeLinkedListNode<T, numChildren>* cweeLinkedListNode<T, numChildren>::GetNext() const {
	return next;
};


#pragma endregion

#pragma region "cweeLinkedList definitions"

template <typename T, int numChildren>
cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::operator=(const cweeLinkedList<T, numChildren>& other) {
	Clear();

	SetGranularity(cweeMath::max(16, other.GetGranularity()));

	for (auto& x : other)
		Append(x);

	return *this;
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::operator==(const T& other) {
	return Find(other);
};

template <typename T, int numChildren>
cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::Reverse() {
#if 1

	int n = NumRef();
	for (int i = 0; i < n; i++) { this->Append(this->operator[](i)); } // copies all of the data in the list to the end of the list, in order
	for (int i = 0; i < n; i++) { this->RemoveIndexFast(i); } // replaces the objects in the initial list with the opposite end's value
	return *this;

#else	
	cweeThreadedList<T> list = ToList().Reverse();
	int i = 0;
	for (auto& x : *this) {
		x = list[i];
		i++;
	}
	return *this;
#endif
};

template <typename T, int numChildren>
void						cweeLinkedList<T, numChildren>::Clear() {
	data.Clear();
	allocator.Shutdown();
	num = 0;
	currentIndex = 0;
};

template <typename T, int numChildren>
void						cweeLinkedList<T, numChildren>::SetGranularity(int newgranularity) {
	data.SetGranularity(cweeMath::max(16, newgranularity / 100));
};				// set new granularity

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::GetGranularity() const {
	return data.GetGranularity() * 100;
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::Num() const {
	return num;
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::NumRef() const {
	return num;
};

template <typename T, int numChildren>
size_t						cweeLinkedList<T, numChildren>::Size() const {
	size_t out = data.Size();
	for (cweeLinkedListNode<T, numChildren>* node : data) {
		out += node->Data().Size();
	}
	return out;
};

template <typename T, int numChildren>
void						cweeLinkedList<T, numChildren>::Condense() {
	for (cweeLinkedListNode<T, numChildren>* ptr : data) {
		ptr->Data().Condense();
	};

	data.Condense();
};

template <typename T, int numChildren>
const T& cweeLinkedList<T, numChildren>::operator[](int index) const {
	if (index < num) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
		return node->absoluteIndex(index);
	}
	else {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(num - 1);
		return node->absoluteIndex(num - 1);
	}
};

template <typename T, int numChildren>
T& cweeLinkedList<T, numChildren>::operator[](int index) {
	if (index < num) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
		return node->absoluteIndex(index);
	}
	else {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(num - 1);
		return node->absoluteIndex(num - 1);
	}
};

template <typename T, int numChildren>
cweeThreadedList<T>			cweeLinkedList<T, numChildren>::ToList() const {
	cweeThreadedList<T> out(num + 16);
	for (cweeLinkedListNode<T, numChildren>* node : data) {
		for (auto& x : node->Data()) {
			out.Append(x);
		}
	}
	return out;
};

template <typename T, int numChildren>
cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::FromList(const cweeThreadedList<T>& inbound) {
	Clear();
	SetGranularity(inbound.GetGranularity() + 16);
	for (auto& x : inbound) {
		Append(x);
	}
	return *this;
};

template <typename T, int numChildren>
cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::AppendList(const cweeThreadedList<T>& inbound) {
	if (GetGranularity() < inbound.GetGranularity() + 16) SetGranularity(inbound.GetGranularity() + 16);
	for (auto& x : inbound) {
		Append(x);
	}
	return *this;
};

template <typename T, int numChildren>
T& cweeLinkedList<T, numChildren>::Alloc() {
	return operator[](Append());
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::Append() {
	if (data.NumRef() <= 0) AppendNode();
	data[data.NumRef() - 1]->Append();
	return num++;
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::Append(const T& obj) {
	if (data.NumRef() <= 0) AppendNode();
	data[data.NumRef() - 1]->Append(obj);
	return num++;
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::AddUnique(const T& obj) {
	int i;
	T* ptr = nullptr;
	i = FindIndex(obj, &ptr);
	if (i >= 0) {
		*ptr = obj;
		return i;
	}
	return Append(obj);
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::Insert(const T& obj, int index) {
	if (index >= num)
		return Append(obj);

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
	node->Insert(obj, index - node->MinIndex());
	return num++;
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::Emplace(const T& obj, int index) {
	while (index >= num) {
		Append(T());
	}
	operator[](index) = obj;
	return index;
};		// insert the element at the given index

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::FindIndex(const T& obj, T** ptr) const {
	int i = 0;
	for (auto& x : *this) {
		if (x == obj) {
			if (ptr) *ptr = const_cast<T*>(&x);
			return i;
		}
		i++;
	}
	return -1;
};

template <typename T, int numChildren>
T* cweeLinkedList<T, numChildren>::Find(T const& obj) const {
	int i;
	T* ptr = nullptr;
	i = FindIndex(obj, &ptr);
	if (i >= 0) {
		return ptr;
	}
	return ptr;
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::RemoveIndex(int index) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->Remove(index - node->MinIndex())) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
void						cweeLinkedList<T, numChildren>::RemoveIndexes(const cweeThreadedList<int>& indexes) {
	if (indexes.NumRef() <= 0) return; // nothing to remove.

	std::vector<int> temp = indexes;
	std::sort(temp.begin(), temp.end());
	temp.push_back(-1);

	num -= indexes.NumRef();
	if (num <= 0) {
		Clear();
		return;
	}

	int removalProgress = 1;
	int currentRemoveIndex = temp[0];
	int copyIndex = currentRemoveIndex + 1;
	int nextRemoveIndex = temp[removalProgress];
	int sizeRemove = indexes.NumRef();
	for (int i = currentRemoveIndex; i < num;) {
		if (nextRemoveIndex > copyIndex || nextRemoveIndex == -1) {
			operator[](i) = operator[](copyIndex);
			copyIndex++;
		}
		else { // the next copy location is actually a removal location
			copyIndex++;
			removalProgress++;
			nextRemoveIndex = temp[removalProgress];
			continue;
		}
		i++;
	}

	for (int i = num + indexes.NumRef() - 1; i >= num; i--) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(i);
		node->Remove(i - node->MinIndex());
	}
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::RemoveIndexFast(int index) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->RemoveFast(index - node->MinIndex())) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::Remove(const T& obj) {
	int f = FindIndex(obj);
	if (f >= 0)
		RemoveIndex(f);
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::RemoveFast(const T& obj) {
	int f = FindIndex(obj);
	if (f >= 0)
		RemoveIndexFast(f);
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::ExtractIndex(int index, T& out) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->Extract(index - node->MinIndex(), out)) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
bool						cweeLinkedList<T, numChildren>::ExtractIndexFast(int index, T& out) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->ExtractFast(index - node->MinIndex(), out)) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::Sort() {
	auto list = ToList();
	list.Sort<T, TAG_STRING>();
	FromList(list);
	return *this;
};

template <typename T, int numChildren>
cweeThreadedList<const T*> cweeLinkedList<T, numChildren>::Select(std::function<bool(const T&)> predicate) const {
	cweeThreadedList<const T*> out;

	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(&operator[](i));
		}
	}
	return out;
};

template <typename T, int numChildren>
cweeThreadedList<T*> cweeLinkedList<T, numChildren>::Select(std::function<bool(const T&)> predicate) {
	cweeThreadedList<T*> out;
	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(&operator[](i));
		}
	}
	return out;
};

template <typename T, int numChildren>
cweeThreadedList<int> cweeLinkedList<T, numChildren>::SelectIndexes(std::function<bool(const T&)> predicate) const {
	cweeThreadedList<int> out;
	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(i);
		}
	}
	return out;
};


template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::InsertNode(int nodeIndex) {
	cweeLinkedListNode<T, numChildren>* out = allocator.Alloc();

	*out = cweeLinkedListNode<T, numChildren>(this, nodeIndex, cweeMath::max(childrenPerNode, num / 100));

	data.Insert(out, nodeIndex);

	return out;
};

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::AppendNode() {
	cweeLinkedListNode<T, numChildren>* out = allocator.Alloc();

	*out = cweeLinkedListNode<T, numChildren>(this, data.NumRef(), cweeMath::max(childrenPerNode, num / 100));

	data.Append(out);

	return out;
};

template <typename T, int numChildren>
void						cweeLinkedList<T, numChildren>::FreeNode(int nodeIndex) {
	cweeLinkedListNode<T, numChildren>* out = data[nodeIndex];
	data.RemoveIndex(nodeIndex);
	allocator.Free(out);
};

template <typename T, int numChildren>
int							cweeLinkedList<T, numChildren>::NumNodes() const {
	return data.NumRef();
};

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::GetNode(int index) {
	return data[index];
};

template <typename T, int numChildren>
const cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::GetNode(int index) const {
	return data[index];
};

template <typename T, int numChildren>
cweeLinkedListNode<T, numChildren>*	cweeLinkedList<T, numChildren>::FindNodeWithIndex(int index) const {
	// binary search the list
	if (data.NumRef() == 1) return data[0];

	int len, mid, offset, t;
	bool res;
	cweeLinkedListNode<T, numChildren>* sample = nullptr;

#if 0

	len = data.NumRef();	
	offset = 0;
	
	mid = index;
	mid /= numChildren;
	mid = cweeMath::min(mid, len - 1);
	sample = data[mid];

	while (sample && mid >= 0)
	{
		if (sample->MinIndex() <= index) {
			// the index is to the right of (or on top of) this sample's minimum
			if (sample->GetNext() && sample->GetNext()->MinIndex() <= index) {
				// the index is upstream of this sample and is not within this sample
				// move to the right
				offset = mid + 1;
				t = len - offset;
				t = t >> 1;
				if (!t) {
					t = 1;
				}				
				mid += t;
							
			}
			else {
				return sample;
			}
		}
		else {
			// the index is to the left of this sample and is not within this sample 
			// move to the left
			len = mid - 1;
			t = len - offset;
			t = t >> 1;
			if (!t) {
				t = 1;
			}
			mid -= t; 					
		}
		sample = data[mid];
	}
	return sample;

#else
	// check the currentIndex (optimization)
	if (currentIndex < data.NumRef() && currentIndex >= 0) {
		sample = data[currentIndex];
		len = sample->MinIndex();
		len += sample->NumRef();

		if (len > index) {
			if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
				return sample;
			}
			else if (index >= (sample->MinIndex() - 1)) { // user is navigating backwards down the chain with the [] operator by -1 at a time
				currentIndex--;
#if 0
				return FindNodeWithIndex(index);
#else
				if (currentIndex < data.NumRef() && currentIndex >= 0) {
					sample = data[currentIndex];
					len = sample->MinIndex();
					len += sample->NumRef();

					if (len > index) {
						if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
							return sample;
						}
					}
				}
#endif
			}
		}
		else if ((len + 1) > index) { // user is navigating up the chain with [] operator by 1 at a time
			currentIndex++;
#if 0
			return FindNodeWithIndex(index);
#else
			if (currentIndex < data.NumRef() && currentIndex >= 0) {
				sample = data[currentIndex];
				len = sample->MinIndex();
				len += sample->NumRef();

				if (len > index) {
					if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
						return sample;
					}
				}
			}
#endif
		}
	}

	// use binary search to find the correct bucket
	{

#if 1 // assumes no knowledge and will begin the bisection in the middle of the list
		len = data.NumRef();
		mid = len;
		offset = 0;
		res = false;

		while (mid > 0) {
			mid = len >> 1;
			sample = data[offset + mid];
			if (index >= sample->MinIndex())
			{
				offset += mid;
				len -= mid;
				res = true;
				if (index < sample->MaxIndex()) {
					currentIndex = offset;
					return sample;
				}
			}
			else
			{
				len -= mid;
				res = false;
			}
		}
		currentIndex = offset + (int)res;
		sample = data[currentIndex];
		return sample;

#else
		len = data.NumRef();
		mid = currentIndex;
		offset = 0;

		mid = index;
		mid /= numChildren;
		mid = cweeMath::min(mid, len - 1);
		sample = data[mid];


		while (sample && mid >= 0)
		{
			if (sample->MinIndex() <= index) {
				// the index is to the right of (or on top of) this sample's minimum
				if (sample->GetNext() && sample->GetNext()->MinIndex() <= index) {
					// the index is upstream of this sample and is not within this sample
					offset = mid + 1;
					t = len - offset;
					t = t >> 1;
					if (!t) t++;
					mid += t; // move to the right
				}
				else {
					// we've arrived
					currentIndex = mid;
					return sample;
				}
			}
			else {
				// the index is to the left of this sample and is not within this sample 
				len = mid - 1; // reduce search space for good. 
				t = len - offset;
				t = t >> 1;
				if (!t) t++;
				mid -= t; // move to the left
			}
			sample = data[mid];
		}
		currentIndex = mid;
		return sample;
#endif
	}
#endif

};

#pragma endregion

#endif