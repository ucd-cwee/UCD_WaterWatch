#pragma once

#pragma once
#include <memory>
#include <set>
#include "atomic_allocator.h"
// #include "atomic_maps.h"

// a fast alternative to the GoodLang::fast_shared_mutex when prioritizing readers over writers. 
namespace GL {
	class fast_shared_mutex {
	private:
		mutable std::atomic<long long> mut{ 0 }; // Read, Write

	public:
		__declspec(noinline) bool try_lock() const {
			thread_local long long read, planned;
			read = planned = mut.load(std::memory_order::memory_order_relaxed);
			if (reinterpret_cast<short*>(&planned)[0] == 0) { // no readers...
				if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...
					if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
						return true; // success!
					}
				}
			}
			return false;
		};
		__declspec(noinline) void unlock() const {
			thread_local long long read, planned;
			int i = 0;
			while (true) {
				if (++i > 40) std::this_thread::yield();
				read = planned = mut.load(std::memory_order::memory_order_relaxed);
				--reinterpret_cast<short*>(&planned)[1];
				if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
					break; // success!
				}
			}
		};
		__declspec(noinline) void lock() const {
			int i = 0;
			while (!try_lock()) {
				if (++i > 40) std::this_thread::yield();
			}
		};

		__declspec(noinline) bool try_lock_shared() const {
			thread_local long long read;
			read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
			if (
				(reinterpret_cast<short*>(&read)[0] >= 1) // we are allowed to read with other readers...
				&& (reinterpret_cast<long*>(&read)[1] == 0) // so long as there are no writers...
				) {
				return true;
			}
			else {
				mut.fetch_add(-1, std::memory_order::memory_order_acq_rel); // failure -- undo our mistake.
				return false;
			}
		};
		__declspec(noinline) void unlock_shared() const {
			mut.fetch_add(-1, std::memory_order::memory_order_acq_rel);
		};
		__declspec(noinline) void lock_shared() const {
			thread_local long long read;
			read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
			while (reinterpret_cast<long*>(&read)[1] != 0) {
				read = mut.load();
			}


			//int i = 0;
			//while (!try_lock_shared()) {
			//	if (++i > 40) std::this_thread::yield();
			//}
		};

		// if you already hold a shared_lock and want to upgrade to a hard lock without releasing.
		// Returns true if this ideal scenario was successful. Returns false otherwise.
		__declspec(noinline) bool upgrade_lock() const {
			thread_local long long read, planned;
			// increment the write count and decrement our read count...
			//for (int i = 0; i < 40; ++i) {
			planned = read = mut.load(std::memory_order::memory_order_relaxed);
			if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...					
				if (--reinterpret_cast<short*>(&planned)[0] == 0) { // we're the only reader...		
					if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
						return true;
					}
				}
			}
			//else {
			//	break;
			//}
		//}

			unlock_shared();
			lock();

			return false;
		};

	};
};


#define CONST_MAX( x, y ) ( (x) > (y) ? (x) : (y) )
namespace GL{
	// Single-threaded version of the B-Tree. Fastest version, but is not thread-safe. Nodes are invalidated if removed.
	template< class objType, class keyType, int maxChildrenPerNode >
	class bTree {
	public:
		struct bTreeNode {
			keyType	// key used for sorting						
				key;
			objType // if != nullptr pointer to object stored in leaf node
				* object;
			bTreeNode // parent node
				* parent;
			bTreeNode // next sibling
				* next;
			bTreeNode // prev sibling
				* prev;
			int	// number of children							
				numChildren;
			bTreeNode // first child
				* firstChild;
			bTreeNode // last child
				* lastChild;
		};

	public:
		bTree() {
			root = AllocNode();
		};
		~bTree() {
			root = nullptr;
		};

		// add an object to the tree
		bTreeNode* Add(objType* object, keyType key) {
			bTreeNode
				* node,
				* child,
				* newNode;

			if (root == nullptr) root = AllocNode();


			if (root->numChildren >= maxChildrenPerNode) {
				newNode = AllocNode();
				newNode->key = root->key;
				newNode->firstChild = root;
				newNode->lastChild = root;
				newNode->numChildren = 1;
				root->parent = newNode;
				SplitNode(root);
				root = newNode;
			}

			newNode = AllocNode();
			newNode->key = key;
			newNode->object = object;

			for (node = root; node->firstChild != nullptr; node = child) {
				if (key > node->key) node->key = key;

				// find the first child with a key larger equal to the key of the new node
				for (child = node->firstChild; child->next; child = child->next)
					if (key <= child->key)
						break;

				if (child->object) {
					if (key <= child->key) {
						// insert new node before child
						if (child->prev) child->prev->next = newNode;
						else node->firstChild = newNode;
						newNode->prev = child->prev;
						newNode->next = child;
						child->prev = newNode;
					}
					else {
						// insert new node after child
						if (child->next) child->next->prev = newNode;
						else node->lastChild = newNode;
						newNode->prev = child;
						newNode->next = child->next;
						child->next = newNode;
					}

					newNode->parent = node;
					node->numChildren++;

					return newNode;
				}

				// make sure the child has room to store another node
				if (child->numChildren >= maxChildrenPerNode) {
					SplitNode(child);
					if (key <= child->prev->key) child = child->prev;
				}
			}

			// we only end up here if the root node is empty
			newNode->parent = root;
			root->key = key;
			root->firstChild = newNode;
			root->lastChild = newNode;
			root->numChildren++;

			return newNode;
		};
		// remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		void Remove(bTreeNode* node) {
			bTreeNode* parent;

			// unlink the node from it's parent
			if (node->prev) node->prev->next = node->next;
			else node->parent->firstChild = node->next;
			if (node->next) node->next->prev = node->prev;
			else node->parent->lastChild = node->prev;
			node->parent->numChildren--;

			// make sure there are no parent nodes with a single child
			for (parent = node->parent; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
				if (parent->next) parent = MergeNodes(parent, parent->next);
				else if (parent->prev) parent = MergeNodes(parent->prev, parent);

				// a parent may not use a key higher than the key of its last child
				if (parent->key > parent->lastChild->key) parent->key = parent->lastChild->key;

				if (parent->numChildren > maxChildrenPerNode) {
					SplitNode(parent);
					break;
				}
			}
			// a parent may not use a key higher than the key of it's last child
			for (; (parent != nullptr) && (parent->lastChild != nullptr); parent = parent->parent)
				if (parent->key > parent->lastChild->key)
					parent->key = parent->lastChild->key;

			// actually free the node
			FreeNode(node);

			// remove the root node if it has a single internal node as child
			if ((root->numChildren == 1) && (root->firstChild->object == nullptr)) {
				bTreeNode* oldRoot = root;
				root->firstChild->parent = nullptr;
				root = root->firstChild;
				FreeNode(oldRoot);
			}
		};
		// find an object using the given key
		bTreeNode* NodeFind(keyType key) const {
			for (bTreeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
				while (node->next) {
					if (node->key >= key) break;
					node = node->next;
				}
				if (node->object) {
					if (node->key == key) return node;
					else return nullptr;
				}
			}
			return nullptr;
		};
		// find an object with the smallest key larger equal the given key
		bTreeNode* NodeFindSmallestLargerEqual(keyType key) const {
			if (root == nullptr) return nullptr;
			for (bTreeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
				while (node->next) {
					if (node->key >= key) break;
					node = node->next;
				}
				if (node->object) {
					if (node->key >= key) return node;
					else return nullptr;
				}
			}
			return nullptr;
		};;
		// find an object with the largest key smaller equal the given key
		bTreeNode* NodeFindLargestSmallerEqual(keyType key) const {
			bTreeNode
				* node,
				* smaller;

			if (root == nullptr) return nullptr;
			for (node = root->firstChild, smaller = nullptr; node != nullptr; node = node->firstChild) {
				while (node->next) {
					if (node->key >= key) break;
					smaller = node;
					node = node->next;
				}
				if (node->object) {
					if (node->key <= key) return node;
					else if (smaller == nullptr) return nullptr;
					else {
						node = smaller;
						if (node->object) return node;
					}
				}
			}
			return nullptr;
		};
		// find an object using the given key
		objType* Find(keyType key) const {
			if (bTreeNode* node = NodeFind(key)) return node->object;
			else return nullptr;
		};
		// find an object with the smallest key larger equal the given key
		objType* FindSmallestLargerEqual(keyType key) const {
			if (bTreeNode* node = NodeFindSmallestLargerEqual(key)) return node->object;
			else return nullptr;
		};
		// find an object with the largest key smaller equal the given key
		objType* FindLargestSmallerEqual(keyType key) const {
			if (bTreeNode* node = NodeFindLargestSmallerEqual(key)) return node->object;
			else return nullptr;
		};
		// returns the root node of the tree
		bTreeNode* GetRoot() const {
			return root;
		};
		// goes through all nodes of the tree
		static bTreeNode* GetNext(bTreeNode* node) {
			if (node->firstChild) return node->firstChild;
			else {
				while (node && (node->next == nullptr)) node = node->parent;
				return node;
			}
		};
		// goes through all leaf nodes of the tree
		static bTreeNode* GetNextLeaf(bTreeNode* node) {
			if (node->firstChild) {
				while (node->firstChild) node = node->firstChild;
				return node;
			}
			else {
				while (node && (node->next == nullptr)) node = node->parent;
				if (node) {
					node = node->next;
					while (node->firstChild) node = node->firstChild;
					return node;
				}
				else return nullptr;
			}
		};

	private:
		bTreeNode* root;
		GL::atomic_allocator< bTreeNode > nodeAllocator;

		bTreeNode*
			AllocNode() {
			bTreeNode* node = nodeAllocator.Alloc();
			node->key = 0;
			node->parent = nullptr;
			node->next = nullptr;
			node->prev = nullptr;
			node->numChildren = 0;
			node->firstChild = nullptr;
			node->lastChild = nullptr;
			node->object = nullptr;
			return node;
		};
		void
			FreeNode(bTreeNode* node) {
			nodeAllocator.Free(node);
		};
		void
			SplitNode(bTreeNode* node) {
			int i;
			bTreeNode* child, * newNode;

			// allocate a new node
			newNode = AllocNode();
			newNode->parent = node->parent;

			// divide the children over the two nodes
			child = node->firstChild;
			child->parent = newNode;
			for (i = 3; i < node->numChildren; i += 2) {
				child = child->next;
				child->parent = newNode;
			}

			newNode->key = child->key;
			newNode->numChildren = node->numChildren / 2;
			newNode->firstChild = node->firstChild;
			newNode->lastChild = child;

			node->numChildren -= newNode->numChildren;
			node->firstChild = child->next;

			child->next->prev = nullptr;
			child->next = nullptr;

			if (node->prev) {
				node->prev->next = newNode;
			}
			else {
				node->parent->firstChild = newNode;
			}
			newNode->prev = node->prev;
			newNode->next = node;
			node->prev = newNode;

			node->parent->numChildren++;
		};;
		bTreeNode*
			MergeNodes(bTreeNode* node1, bTreeNode* node2) {
			bTreeNode* child;

			for (child = node1->firstChild; child->next; child = child->next) {
				child->parent = node2;
			}
			child->parent = node2;
			child->next = node2->firstChild;
			node2->firstChild->prev = child;
			node2->firstChild = node1->firstChild;
			node2->numChildren += node1->numChildren;

			// unlink the first node from the parent
			if (node1->prev) {
				node1->prev->next = node2;
			}
			else {
				node1->parent->firstChild = node2;
			}
			node2->prev = node1->prev;
			node2->parent->numChildren--;

			FreeNode(node1);

			return node2;
		};

	};

	// Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
	// Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
	template< class objType, class keyType, int maxChildrenPerNode >
	class parallel_binary_search_tree {
	public:
		using lock_type = fast_shared_mutex; // std::shared_mutex;
		struct parallel_binary_search_treeNode {
			keyType	// key used for sorting						
				key;
			parallel_binary_search_treeNode // parent node
				* parent;
			int	// number of children							
				numChildren;
			int
				parent_index;
			void* // std::variant< objType*, std::array<parallel_binary_search_treeNode*, maxChildrenPerNode>* >		
				data;
			bool
				is_leaf;
			objType*
				object() {
				if (numChildren > 0 || !is_leaf) return nullptr;
				return static_cast<objType*>(data);
			};
			parallel_binary_search_treeNode**
				children() {
				return &static_cast<std::array<parallel_binary_search_treeNode*, maxChildrenPerNode>*>(data)->operator[](0);
			};

			parallel_binary_search_treeNode* // next sibling
				next() {
				if (parent && (parent->numChildren > (parent_index + 1))) {
					return parent->children()[parent_index + 1];
				}
				else {
					return nullptr;
				}
			};
			parallel_binary_search_treeNode* // prev sibling
				prev() {
				if (parent && (parent_index >= 1)) {
					return parent->children()[parent_index - 1];
				}
				else {
					return nullptr;
				}
			};
			parallel_binary_search_treeNode* // first child
				firstChild() {
				if (numChildren == 0) return nullptr;
				return children()[0];
			};
			parallel_binary_search_treeNode* // last child
				lastChild() {
				if (numChildren == 0) return nullptr;
				return children()[numChildren - 1];
			};
			void
				add_child(parallel_binary_search_treeNode* p) {
				p->parent = this;
				p->parent_index = numChildren;
				children()[numChildren] = p;
				++numChildren;
				if (this->key < p->key) this->key = p->key;
			}
			__declspec(noinline) void
				add_child_at(parallel_binary_search_treeNode* p, int i) {
				if (i >= numChildren) add_child(p);
				else {
					parallel_binary_search_treeNode**
						ch = children();
					int
						j;

					if (this->key < p->key) this->key = p->key;
					p->parent = this;
					p->parent_index = i;
					// shift everything forward

	#if 0
					for (j = numChildren; j > i; --j) {
						ch[j] = ch[j - 1];
						ch[j]->parent_index = j;
					}
					ch[i] = p;
	#else
					std::memmove(&ch[i + 1], &ch[i], sizeof(parallel_binary_search_treeNode*) * (numChildren - i));
					ch[i] = p;
					ch = &ch[i];
					for (j = i + 1; j <= numChildren; ++j) {
						++ch;
						(*ch)->parent_index = j;
					}
	#endif
					++numChildren;
				}
			}
			parallel_binary_search_treeNode*
				pop_front_child() {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					auto* out = children()[0];
					int i = 0;
					for (i = 0; i < (numChildren - 1); ++i) {
						children()[i] = children()[i + 1];
						children()[i]->parent_index = i;
					}
					children()[i] = nullptr;
					--numChildren;
					return out;
				}
			}
			void
				pop_front_children(int n) {
				if (numChildren >= n) {
					int i = 0;
					for (i = 0; i < (numChildren - n); ++i) {
						children()[i] = children()[i + n];
						children()[i]->parent_index = i;
					}
					for (; i < maxChildrenPerNode; ++i) {
						children()[i] = nullptr;
					}
					numChildren -= n;
				}
			}
			parallel_binary_search_treeNode*
				pop_child(int i) {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					parallel_binary_search_treeNode**
						ch = children();
					parallel_binary_search_treeNode*
						out = ch[i];
					int
						j = numChildren - 1;

	#if 0
					for (; i < (numChildren - 1); ++i) {
						ch[i] = ch[i + 1];
						ch[i]->parent_index = i;
					}
					ch[i] = nullptr;
					--numChildren;
					if (numChildren > 0) this->key = ch[numChildren - 1]->key;
	#else
					std::memmove(&ch[i], &ch[i + 1], sizeof(parallel_binary_search_treeNode*) * ((numChildren - i) - 1));
					if (numChildren > 1) this->key = ch[numChildren - 2]->key;
					ch[j] = nullptr;
					ch = &ch[i];
					for (; i < j; ++i) {
						(*ch)->parent_index = i;
						++ch;
					}
					--numChildren;
	#endif
					return out;
				}
			}
			parallel_binary_search_treeNode*
				pop_back_child() {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					auto* out = children()[numChildren - 1];
					children()[numChildren - 1] = nullptr;
					--numChildren;
					if (numChildren > 0) this->key = children()[numChildren - 1]->key;
					return out;
				}
			};
			parallel_binary_search_treeNode*
				binomial_search_smallest_greater_equal_to(keyType K) {
	#if 0
				parallel_binary_search_treeNode* child = this->firstChild();
				for (; child->next(); child = child->next()) {
					if (K <= child->key)
						break;
				}
				return child;
	#else
				//if (K >= children()[numChildren - 1]->key) {
				//	// it will be one of the final children
				//	for (int i = numChildren - 2; i >= 0; --i) {
				//		if (children()[i]->key < K) return children()[i + 1];
				//	}
				//	// worst-case we searched them all...
				//	return children()[0];
				//}
				//else {
				int
					len,
					mid,
					offset;
				bool
					res;
				parallel_binary_search_treeNode
					* sample;
				if (numChildren == 0)
					return nullptr;
				if (numChildren == 1)
					return children()[0];

				len = numChildren;
				mid = len;
				offset = 0;
				res = false;

				while (mid > 0) {
					mid = len >> 1;
					sample = children()[offset + mid];
					if (K >= sample->key) {
						offset += mid;
						len -= mid;
						res = true;
						if (K == sample->key) return sample;
					}
					else {
						len -= mid;
						res = false;
					}
				}
				mid = offset + (int)res;
				if (mid == numChildren) return children()[offset];
				else return children()[mid];
				//}
	#endif
			};

		};

	private:
		lock_type
			mut; // global tree lock. Should only be held temporarily if at all possible. 
		parallel_binary_search_treeNode*
			root;
		GL::atomic_allocator< parallel_binary_search_treeNode, 256, true >
			nodeAllocator;
		GL::atomic_allocator< std::array<parallel_binary_search_treeNode*, maxChildrenPerNode>, 32, true >
			nodeChildrenAllocator;
		long
			count;
	private:
		class // exclusive lock manager. Since this is a course-grained type, though, it can only ever hold one lock at a time. 
			locker {
		public:
			lock_type*
				locked;
			bool
				hard_locked;

			locker() : locked{ nullptr }, hard_locked{ false } {};
			locker(locker const&) = delete;
			locker(locker&& rhs) noexcept : locked{ rhs.locked }, hard_locked{ rhs.hard_locked } { rhs.locked = nullptr; };
			locker& operator=(locker const&) = delete;
			locker& operator=(locker&& rhs) noexcept {
				clear();
				locked = rhs.locked;
				hard_locked = rhs.hard_locked;
				rhs.locked = nullptr;
				return *this;
			};
			~locker() {
				clear();
			};
			operator bool() const {
				return locked != nullptr;
			};

		public:
			__declspec(noinline) bool // store a shared lock
				try_push_back(lock_type& source) {
				clear();
				locked = &source;
				if (locked->try_lock()) {
					hard_locked = true;
					return true;
				}
				else {
					hard_locked = false;
					locked = nullptr;
					return false;
				}
			};
			void // store a shared lock
				push_back(lock_type& source) {
				clear();
				locked = &source;
				locked->lock();
				hard_locked = true;
			};
			void // store a shared lock
				push_back_shared(lock_type& source) {
				clear();
				locked = &source;
				locked->lock_shared();
				hard_locked = false;
			};
			void // store a shared lock
				push_pop(lock_type& source) {
				source.lock();
				clear();
				locked = &source;
				hard_locked = true;
			};
			void // store a shared lock
				push_pop_shared(lock_type& source) {
				source.lock_shared();
				clear();
				locked = &source;
				hard_locked = false;
			};
			void // remove the youngest lock
				pop_back() {
				clear();
			};
			void // remove the oldest lock
				pop_front() {
				clear();
			};
			size_t // count of locks
				size() const {
				return (locked != nullptr) ? 1 : 0;
			};
			void // clear all locks
				clear() {
				if (locked != nullptr) {
					if (hard_locked) locked->unlock();
					else locked->unlock_shared();
					locked = nullptr;
				}
			};
		};
	public:
		parallel_binary_search_tree()
			: nodeAllocator()
			, root{ nullptr }
			, mut()
			, count{ 0 }
		{
			root = AllocNode(false);
		};
		parallel_binary_search_tree(parallel_binary_search_tree const&)
			= delete;
		parallel_binary_search_tree(parallel_binary_search_tree&&) noexcept
			= delete;
		parallel_binary_search_tree& operator=(parallel_binary_search_tree const&)
			= delete;
		parallel_binary_search_tree& operator=(parallel_binary_search_tree&&) noexcept
			= delete;
		~parallel_binary_search_tree()
			= default;

		__declspec(noinline) parallel_binary_search_treeNode* // add an object to the tree
			Add(objType* object, keyType key, locker const& Locking = locker()) {
			parallel_binary_search_treeNode
				* node,
				* child,
				* newNode;
			locker&
				locking = const_cast<locker&>(Locking);
			newNode
				= AllocNode(true);
			newNode->key
				= key;
			newNode->data
				= object;

			if (!locking) {
				locking.push_back(mut); // locked
			}

			if (root == nullptr) root = AllocNode(false); // start fresh
			if (root == nullptr) throw std::runtime_error("Root should not have been nullptr");

			if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
				node = AllocNode(false);
				node->key = root->key;
				node->add_child(root);
				SplitNode(root);
				root = node;
				node = nullptr;
			}

			for (node = root; node->numChildren > 0; node = child) {
				if (key > node->key) node->key = key; // in prep for the insertion

				// find the first child with a key larger equal to the key of the new node
				child = node->binomial_search_smallest_greater_equal_to(key);

				// we are inside of a branch of leafs -- we will do the insert.
				if (child->object()) {
					if (key <= child->key) {
						// insert new node before child
						node->add_child_at(newNode, child->parent_index);
					}
					else {
						// insert new node after child
						node->add_child_at(newNode, child->parent_index + 1);
					}

					++count;
					return newNode;
				}
				else if (child->numChildren >= maxChildrenPerNode) {
					SplitNode(child);
					if (key <= child->prev()->key) child = child->prev();
				}
			}

			// we only end up here if the root node is empty
			root->add_child(newNode);

			++count;
			return newNode;

		};
		__declspec(noinline) bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
			Remove(parallel_binary_search_treeNode* node, locker const& Locking = locker()) {
			parallel_binary_search_treeNode
				* Node,
				* parent,
				* oldRoot;
			locker&
				locking = const_cast<locker&>(Locking);

			// acquire all relevant locks before we perform the deletion
			if (locking.size() == 0) locking.push_back(mut); // get the global tree lock		

			// unlink the node from it's parent
			parent = node->parent;
			parent->pop_child(node->parent_index);

			// make sure there are no parent nodes with a single child
			for (; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
				while (true) {
					if (Node = parent->next()) {
						if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
							SplitNode(Node);
							continue;
						}
						parent = MergeNodes(parent, Node);
						break;
					}
					else if (Node = parent->prev()) {
						if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
							SplitNode(Node);
							continue;
						}
						parent = MergeNodes(Node, parent);
						break;
					}
				}

				if (parent->numChildren > maxChildrenPerNode) {
					SplitNode(parent);
					break;
				}
			}

			// a parent may not use a key higher than the key of it's last child. Work backwards and make sure this is true. 
			for (; parent && (parent->numChildren > 0); parent = parent->parent)
				if (Node = parent->children()[parent->numChildren - 1])
					if (parent->key > Node->key)
						parent->key = Node->key;

			// actually free the node
			--count;
			FreeNode(node);

			// remove the root node if it has a single internal node as child		
			if ((root->numChildren == 1) && !root->firstChild()->object()) {
				oldRoot = root;

				root = oldRoot->firstChild();
				root->parent = nullptr;
				root->parent_index = 0;

				FreeNode(oldRoot);
			}

			return true;
		};
		std::pair<parallel_binary_search_treeNode*, locker> // find an object using the given key
			NodeFind(keyType key, bool for_removal = false) {
			parallel_binary_search_treeNode
				* nxt;
			std::pair<parallel_binary_search_treeNode*, locker>
				out;
			parallel_binary_search_treeNode*&
				node = out.first;
			locker&
				locking = out.second;

			if (!for_removal) locking.push_back_shared(mut);
			else locking.push_back(mut);
			if (!root || (root->numChildren <= 0)) {
				node = nullptr;
				locking.clear();
				return out;
			}

			for (node = root; node; ) {
				node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
				if (node->object()) {
					if (node->key == key) return out;
					else {
						node = nullptr;
						locking.clear();
						return out;
					}
				}
				if (!node || (node->numChildren <= 0)) {
					node = nullptr;
					locking.clear();
					return out;
				}
			}
			node = nullptr;
			locking.clear();
			return out;
		};
		std::pair<parallel_binary_search_treeNode*, locker> // find an object using the given key
			NodeFind_ForRemoval(keyType key) {
			return NodeFind(key, true);
		};
		locker
			try_lock() {
			locker out;
			out.try_push_back(mut);
			return out;
		};
		locker
			lock() {
			locker out;
			out.push_back(mut);
			return out;
		};

		std::pair<parallel_binary_search_treeNode*, locker> // find an object with the smallest key larger equal the given key
			NodeFindSmallestLargerEqual(keyType key, bool for_removal = false) {
			std::pair<parallel_binary_search_treeNode*, locker>
				out;
			parallel_binary_search_treeNode*&
				node = out.first;
			locker&
				locking = out.second;

			if (!for_removal) locking.push_back_shared(mut);
			else locking.push_back(mut);
			if (!root || (root->numChildren <= 0)) {
				node = nullptr;
				locking.clear();
				return out;
			}
			for (node = root; node; ) {
				node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
				if (node->object()) {
					if (node->key >= key) return out;
					else {
						node = nullptr;
						locking.clear();
						return out;
					}
				}
				if (!node || (node->numChildren <= 0)) {
					node = nullptr;
					locking.clear();
					return out;
				}
			}
			node = nullptr;
			locking.clear();
			return out;
		};
		parallel_binary_search_treeNode* // find an object with the smallest key larger equal the given key
			NodeFindSmallestLargerEqual_Locked(keyType key, locker const& locked) {
			parallel_binary_search_treeNode*
				node = nullptr;

			if (!root || (root->numChildren <= 0)) {
				node = nullptr;
				return node;
			}
			for (node = root; node; ) {
				node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
				if (node->object()) {
					if (node->key >= key) return node;
					else {
						node = nullptr;
						return node;
					}
				}
				if (!node || (node->numChildren <= 0)) {
					node = nullptr;
					return node;
				}
			}
			node = nullptr;
			return node;
		};
		std::pair<parallel_binary_search_treeNode*, locker> // find an object with the smallest key larger equal the given key
			NodeFindSmallestLargerEqual_ForRemoval(keyType key) {
			return NodeFindSmallestLargerEqual(key, true);
		}


	#if 0
		std::pair<parallel_binary_search_treeNode*, locker> // find an object with the largest key smaller equal the given key
			NodeFindLargestSmallerEqual(keyType key) {
			parallel_binary_search_treeNode
				* smaller;
			std::pair<parallel_binary_search_treeNode*, locker>
				out;
			parallel_binary_search_treeNode*&
				node = out.first;
			locker&
				locking = out.second;

			locking.push_back(mut);
			if (!root || !root->firstChild) {
				node = nullptr;
				locking.clear();
				return out;
			}

			for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
				while (node->next) {
					if (node->key >= key) break;
					smaller = node;
					node = node->next;
				}
				if (node->object) {
					if (node->key <= key) return node;
					else if (smaller == nullptr) {
						node = nullptr;
						locking.clear();
						return out;
					}
					else {
						node = smaller;
						if (node->object) return out;
					}
				}

				if (!node || !node->firstChild) {
					node = nullptr;
					locking.clear();
					return out;
				}

				node = node->firstChild;
			}
			node = nullptr;
			locking.clear();
			return out;
		};
	#endif
		std::pair<parallel_binary_search_treeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
			GetRoot() {
			std::pair<parallel_binary_search_treeNode*, locker> out;
			out.second.push_back(mut);
			out.first = root;
			return out;
		};
		parallel_binary_search_treeNode* // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
			GetRoot(locker const& locked) {
			return root;
		};
	#if 0
		static parallel_binary_search_treeNode* // goes through all nodes of the tree		
			GetNext(parallel_binary_search_treeNode* node, locker& locking) {
			if (node->firstChild) {
				return node->firstChild;
			}
			else {
				while (node && (node->next == nullptr)) {
					node = node->parent;
				}
				return node;
			}

		};
	#endif
		static parallel_binary_search_treeNode* // goes through all leaf nodes of the tree		
			GetNextLeaf(parallel_binary_search_treeNode* node, locker& locking) {
			if (!node) return nullptr;

			parallel_binary_search_treeNode*
				nxt;
			if (nxt = node->firstChild()) {
				while (nxt) {
					node = nxt;
					nxt = node->firstChild();
				}
				return node;
			}
			else {
				while (node && (node->next() == nullptr)) {
					nxt = node->parent;
					node = nxt;
				}
				if (node) {
					nxt = node->next();
					node = nxt;
					nxt = node->firstChild();
					while (nxt) {
						node = nxt;
						nxt = node->firstChild();
					}
					return node;
				}
				else return nullptr;
			}
		};
		size_t size() const {
			auto locked = std::shared_lock(mut);
			return (size_t)count;
		};

	private:
		parallel_binary_search_treeNode*
			AllocNode(bool is_leaf) {
			parallel_binary_search_treeNode
				* node;

			node = nodeAllocator.Alloc();
			if (is_leaf) {
				node->is_leaf = true;
				node->data = nullptr;
			}
			else {
				node->is_leaf = false;
				node->data = nodeChildrenAllocator.Alloc();
				for (int i = 0; i < maxChildrenPerNode; ++i) node->children()[i] = nullptr;
			}

			node->key = 0;
			node->parent = nullptr;
			node->parent_index = 0;
			node->numChildren = 0;

			return node;
		};
		__declspec(noinline) void
			FreeNode(parallel_binary_search_treeNode* node) {
			if (node) {
				if (!node->is_leaf) {
					nodeChildrenAllocator.Free(static_cast<std::array<parallel_binary_search_treeNode*, maxChildrenPerNode>*>(node->data));
				}
				nodeAllocator.Free(node);
			}
		};
		void // will split node by creating a neighbor next to it in the parent node and sharing half its children
			SplitNode(parallel_binary_search_treeNode* node) {
			int
				i, j;
			parallel_binary_search_treeNode
				* child,
				* newNode;

			// allocate a new node
			newNode = AllocNode(false);
			newNode->parent = node->parent;

			// divide the children over the two nodes
			child = node->firstChild();
			newNode->children()[0] = child;
			for (j = 1, i = 3; i < node->numChildren; i += 2, j++) {
				child = child->next();
				newNode->children()[j] = child;
			}
			newNode->key = child->key;
			newNode->numChildren = node->numChildren / 2;
			for (i = 0; i < newNode->numChildren; ++i) {
				newNode->children()[i]->parent = newNode;
				newNode->children()[i]->parent_index = i;
			}

			newNode->parent_index = node->parent_index;
			node->pop_front_children(newNode->numChildren);
			node->parent->add_child_at(newNode, newNode->parent_index);
			node->key = node->children()[node->numChildren - 1]->key;
		};;
		parallel_binary_search_treeNode* // node1 will be deleted and its children appended to node2
			MergeNodes(parallel_binary_search_treeNode* node1, parallel_binary_search_treeNode* node2) {
			for (int i = 0; i < node1->numChildren; ++i)
				node2->add_child_at(node1->children()[i], i);
			(void)node1->parent->pop_child(node1->parent_index);
			FreeNode(node1);
			return node2;
		};

	};

	// Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
	// Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
	template< class objType, class keyType, int maxChildrenPerNode >
	class binary_search_tree {
	public:
		struct binary_search_treeNode {
			keyType	// key used for sorting						
				key;
			binary_search_treeNode // parent node
				* parent;
			int	// number of children							
				numChildren;
			int
				parent_index;
			void* // std::variant< objType*, std::array<binary_search_treeNode*, maxChildrenPerNode>* >		
				data;
			bool
				is_leaf;
			objType*
				object() {
				if (numChildren > 0 || !is_leaf) return nullptr;
				return static_cast<objType*>(data);
			};
			binary_search_treeNode**
				children() {
				return &static_cast<std::array<binary_search_treeNode*, maxChildrenPerNode>*>(data)->operator[](0);
			};

			binary_search_treeNode* // next sibling
				next() {
				if (parent && (parent->numChildren > (parent_index + 1))) {
					return parent->children()[parent_index + 1];
				}
				else {
					return nullptr;
				}
			};
			binary_search_treeNode* // prev sibling
				prev() {
				if (parent && (parent_index >= 1)) {
					return parent->children()[parent_index - 1];
				}
				else {
					return nullptr;
				}
			};
			binary_search_treeNode* // first child
				firstChild() {
				if (numChildren == 0) return nullptr;
				return children()[0];
			};
			binary_search_treeNode* // last child
				lastChild() {
				if (numChildren == 0) return nullptr;
				return children()[numChildren - 1];
			};
			void
				add_child(binary_search_treeNode* p) {
				p->parent = this;
				p->parent_index = numChildren;
				children()[numChildren] = p;
				++numChildren;
				if (this->key < p->key) this->key = p->key;
			}
			__declspec(noinline) void
				add_child_at(binary_search_treeNode* p, int i) {
				if (i >= numChildren) add_child(p);
				else {
					binary_search_treeNode**
						ch = children();
					int
						j;

					if (this->key < p->key) this->key = p->key;
					p->parent = this;
					p->parent_index = i;
					// shift everything forward

	#if 0
					for (j = numChildren; j > i; --j) {
						ch[j] = ch[j - 1];
						ch[j]->parent_index = j;
					}
					ch[i] = p;
	#else
					std::memmove(&ch[i + 1], &ch[i], sizeof(binary_search_treeNode*) * (numChildren - i));
					ch[i] = p;
					ch = &ch[i];
					for (j = i + 1; j <= numChildren; ++j) {
						++ch;
						(*ch)->parent_index = j;
					}
	#endif
					++numChildren;
				}
			}
			binary_search_treeNode*
				pop_front_child() {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					auto* out = children()[0];
					int i = 0;
					for (i = 0; i < (numChildren - 1); ++i) {
						children()[i] = children()[i + 1];
						children()[i]->parent_index = i;
					}
					children()[i] = nullptr;
					--numChildren;
					return out;
				}
			}
			void
				pop_front_children(int n) {
				if (numChildren >= n) {
					int i = 0;
					for (i = 0; i < (numChildren - n); ++i) {
						children()[i] = children()[i + n];
						children()[i]->parent_index = i;
					}
					for (; i < maxChildrenPerNode; ++i) {
						children()[i] = nullptr;
					}
					numChildren -= n;
				}
			}
			binary_search_treeNode*
				pop_child(int i) {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					binary_search_treeNode**
						ch = children();
					binary_search_treeNode*
						out = ch[i];
					int
						j = numChildren - 1;

	#if 0
					for (; i < (numChildren - 1); ++i) {
						ch[i] = ch[i + 1];
						ch[i]->parent_index = i;
					}
					ch[i] = nullptr;
					--numChildren;
					if (numChildren > 0) this->key = ch[numChildren - 1]->key;
	#else
					std::memmove(&ch[i], &ch[i + 1], sizeof(binary_search_treeNode*) * ((numChildren - i) - 1));
					if (numChildren > 1) this->key = ch[numChildren - 2]->key;
					ch[j] = nullptr;
					ch = &ch[i];
					for (; i < j; ++i) {
						(*ch)->parent_index = i;
						++ch;
					}
					--numChildren;
	#endif
					return out;
				}
			}
			binary_search_treeNode*
				pop_back_child() {
				if (numChildren <= 0) {
					return nullptr;
				}
				else {
					auto* out = children()[numChildren - 1];
					children()[numChildren - 1] = nullptr;
					--numChildren;
					if (numChildren > 0) this->key = children()[numChildren - 1]->key;
					return out;
				}
			};
			binary_search_treeNode*
				binomial_search_smallest_greater_equal_to(keyType K) {
	#if 0
				binary_search_treeNode* child = this->firstChild();
				for (; child->next(); child = child->next()) {
					if (K <= child->key)
						break;
				}
				return child;
	#else
				//if (K >= children()[numChildren - 1]->key) {
				//	// it will be one of the final children
				//	for (int i = numChildren - 2; i >= 0; --i) {
				//		if (children()[i]->key < K) return children()[i + 1];
				//	}
				//	// worst-case we searched them all...
				//	return children()[0];
				//}
				//else {
				int
					len,
					mid,
					offset;
				bool
					res;
				binary_search_treeNode
					* sample;
				if (numChildren == 0)
					return nullptr;
				if (numChildren == 1)
					return children()[0];

				len = numChildren;
				mid = len;
				offset = 0;
				res = false;

				while (mid > 0) {
					mid = len >> 1;
					sample = children()[offset + mid];
					if (K >= sample->key) {
						offset += mid;
						len -= mid;
						res = true;
						if (K == sample->key) return sample;
					}
					else {
						len -= mid;
						res = false;
					}
				}
				mid = offset + (int)res;
				if (mid == numChildren) return children()[offset];
				else return children()[mid];
				//}
	#endif
			};

		};

	private:
		binary_search_treeNode*
			root;
		GL::atomic_allocator< binary_search_treeNode, 256, true >
			nodeAllocator;
		GL::atomic_allocator< std::array<binary_search_treeNode*, maxChildrenPerNode>, 32, true >
			nodeChildrenAllocator;
		long
			count;

	public:
		binary_search_tree()
			: nodeAllocator()
			, root{ nullptr }
			, count{ 0 }
		{
			root = AllocNode(false);
		};
		binary_search_tree(binary_search_tree const&)
			= delete;
		binary_search_tree(binary_search_tree&&) noexcept
			= delete;
		binary_search_tree& operator=(binary_search_tree const&)
			= delete;
		binary_search_tree& operator=(binary_search_tree&&) noexcept
			= delete;
		~binary_search_tree()
			= default;

		__declspec(noinline) binary_search_treeNode* // add an object to the tree
			Add(objType* object, keyType key) {
			binary_search_treeNode
				* node,
				* child,
				* newNode;
			newNode
				= AllocNode(true);
			newNode->key
				= key;
			newNode->data
				= object;

			if (root == nullptr) root = AllocNode(false); // start fresh

			if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
				node = AllocNode(false);
				node->key = root->key;
				node->add_child(root);
				SplitNode(root);
				root = node;
				node = nullptr;
			}

			for (node = root; node->numChildren > 0; node = child) {
				if (key > node->key) node->key = key; // in prep for the insertion

				// find the first child with a key larger equal to the key of the new node
				child = node->binomial_search_smallest_greater_equal_to(key);

				// we are inside of a branch of leafs -- we will do the insert.
				if (child->object()) {
					if (key <= child->key) {
						// insert new node before child
						node->add_child_at(newNode, child->parent_index);
					}
					else {
						// insert new node after child
						node->add_child_at(newNode, child->parent_index + 1);
					}

					++count;
					return newNode;
				}
				else if (child->numChildren >= maxChildrenPerNode) {
					SplitNode(child);
					if (key <= child->prev()->key) child = child->prev();
				}
			}

			// we only end up here if the root node is empty
			root->add_child(newNode);

			++count;
			return newNode;

		};
		__declspec(noinline) bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
			Remove(binary_search_treeNode* node) {
			binary_search_treeNode
				* Node,
				* parent,
				* oldRoot;

			// unlink the node from it's parent
			parent = node->parent;
			parent->pop_child(node->parent_index);

			// make sure there are no parent nodes with a single child
			for (; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
				while (true) {
					if (Node = parent->next()) {
						if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
							SplitNode(Node);
							continue;
						}
						parent = MergeNodes(parent, Node);
						break;
					}
					else if (Node = parent->prev()) {
						if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
							SplitNode(Node);
							continue;
						}
						parent = MergeNodes(Node, parent);
						break;
					}
				}

				if (parent->numChildren > maxChildrenPerNode) {
					SplitNode(parent);
					break;
				}
			}

			// a parent may not use a key higher than the key of it's last child. Work backwards and make sure this is true. 
			for (; parent && (parent->numChildren > 0); parent = parent->parent)
				if (Node = parent->children()[parent->numChildren - 1])
					if (parent->key > Node->key)
						parent->key = Node->key;

			// actually free the node
			--count;
			FreeNode(node);

			// remove the root node if it has a single internal node as child		
			if ((root->numChildren == 1) && !root->firstChild()->object()) {
				oldRoot = root;

				root = oldRoot->firstChild();
				root->parent = nullptr;
				root->parent_index = 0;

				FreeNode(oldRoot);
			}

			return true;
		};
		binary_search_treeNode* // find an object using the given key
			NodeFind(keyType key, bool for_removal = false) {
			binary_search_treeNode
				* nxt;
			binary_search_treeNode*
				node;

			if (!root || (root->numChildren <= 0)) return nullptr;

			for (node = root; node; ) {
				node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
				if (node->object()) {
					if (node->key == key) return node;
					else return nullptr;
				}
				if (!node || (node->numChildren <= 0)) return nullptr;
			}
			return nullptr;
		};

		binary_search_treeNode* // find an object with the smallest key larger equal the given key
			NodeFindSmallestLargerEqual(keyType key, bool for_removal = false) {
			binary_search_treeNode
				* nxt;
			binary_search_treeNode*
				node;

			if (!root || (root->numChildren <= 0)) return nullptr;
			for (node = root; node; ) {
				node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
				if (node->object()) {
					if (node->key >= key) return node;
					else return nullptr;
				}
				if (!node || (node->numChildren <= 0)) return nullptr;
			}
			return nullptr;
		};

	#if 0
		std::pair<binary_search_treeNode*, locker> // find an object with the largest key smaller equal the given key
			NodeFindLargestSmallerEqual(keyType key) {
			binary_search_treeNode
				* smaller;
			std::pair<binary_search_treeNode*, locker>
				out;
			binary_search_treeNode*&
				node = out.first;
			locker&
				locking = out.second;

			locking.push_back(mut);
			if (!root || !root->firstChild) {
				node = nullptr;
				locking.clear();
				return out;
			}

			for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
				while (node->next) {
					if (node->key >= key) break;
					smaller = node;
					node = node->next;
				}
				if (node->object) {
					if (node->key <= key) return node;
					else if (smaller == nullptr) {
						node = nullptr;
						locking.clear();
						return out;
					}
					else {
						node = smaller;
						if (node->object) return out;
					}
				}

				if (!node || !node->firstChild) {
					node = nullptr;
					locking.clear();
					return out;
				}

				node = node->firstChild;
			}
			node = nullptr;
			locking.clear();
			return out;
		};
	#endif
		binary_search_treeNode* // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
			GetRoot() {
			return root;
		};
	#if 0
		static binary_search_treeNode* // goes through all nodes of the tree		
			GetNext(binary_search_treeNode* node, locker& locking) {
			if (node->firstChild) {
				return node->firstChild;
			}
			else {
				while (node && (node->next == nullptr)) {
					node = node->parent;
				}
				return node;
			}

		};
	#endif
		static binary_search_treeNode* // goes through all leaf nodes of the tree		
			GetNextLeaf(binary_search_treeNode* node) {
			if (!node) return nullptr;

			binary_search_treeNode*
				nxt;
			if (nxt = node->firstChild()) {
				while (nxt) {
					node = nxt;
					nxt = node->firstChild();
				}
				return node;
			}
			else {
				while (node && (node->next() == nullptr)) {
					nxt = node->parent;
					node = nxt;
				}
				if (node) {
					nxt = node->next();
					node = nxt;
					nxt = node->firstChild();
					while (nxt) {
						node = nxt;
						nxt = node->firstChild();
					}
					return node;
				}
				else return nullptr;
			}
		};
		size_t
			size() const {
			return (size_t)count;
		};

	private:
		binary_search_treeNode*
			AllocNode(bool is_leaf) {
			binary_search_treeNode
				* node;

			node = nodeAllocator.Alloc();
			if (is_leaf) {
				node->is_leaf = true;
				node->data = nullptr;
			}
			else {
				node->is_leaf = false;
				node->data = nodeChildrenAllocator.Alloc();
				for (int i = 0; i < maxChildrenPerNode; ++i) node->children()[i] = nullptr;
			}

			node->key = 0;
			node->parent = nullptr;
			node->parent_index = 0;
			node->numChildren = 0;

			return node;
		};
		__declspec(noinline) void
			FreeNode(binary_search_treeNode* node) {
			if (node) {
				if (!node->is_leaf) {
					nodeChildrenAllocator.Free(static_cast<std::array<binary_search_treeNode*, maxChildrenPerNode>*>(node->data));
				}
				nodeAllocator.Free(node);
			}
		};
		void // will split node by creating a neighbor next to it in the parent node and sharing half its children
			SplitNode(binary_search_treeNode* node) {
			int
				i, j;
			binary_search_treeNode
				* child,
				* newNode;

			// allocate a new node
			newNode = AllocNode(false);
			newNode->parent = node->parent;

			// divide the children over the two nodes
			child = node->firstChild();
			newNode->children()[0] = child;
			for (j = 1, i = 3; i < node->numChildren; i += 2, j++) {
				child = child->next();
				newNode->children()[j] = child;
			}
			newNode->key = child->key;
			newNode->numChildren = node->numChildren / 2;
			for (i = 0; i < newNode->numChildren; ++i) {
				newNode->children()[i]->parent = newNode;
				newNode->children()[i]->parent_index = i;
			}

			newNode->parent_index = node->parent_index;
			node->pop_front_children(newNode->numChildren);
			node->parent->add_child_at(newNode, newNode->parent_index);
			node->key = node->children()[node->numChildren - 1]->key;
		};;
		binary_search_treeNode* // node1 will be deleted and its children appended to node2
			MergeNodes(binary_search_treeNode* node1, binary_search_treeNode* node2) {
			for (int i = 0; i < node1->numChildren; ++i)
				node2->add_child_at(node1->children()[i], i);
			(void)node1->parent->pop_child(node1->parent_index);
			FreeNode(node1);
			return node2;
		};

	};
};
#undef CONST_MAX

namespace GL {
	// thread-safe memory manager. Re-using previous allocations is prioritized, but will swiftly release memory if not needed anymore. 
	template <typename buffer_type = void*>
	class dynamic_allocator {
	public:
		struct dynamic_block {
			buffer_type // this sub-buffer may NOT be further split. It should instead be free'd, then a new sub-buffer generated from the original "real" buffer. 
				sub_buffer;
			unsigned long long // the blocks are sorted by this length... that is how we quickly find buffers of adequate size for the request. 
				length;
			long
				locker;
			long // this sub-buffer may be further split. 
				parent_buffer;
			unsigned int
				thread_id;
			bool
				is_available;
			bool
				is_free;

			__declspec(noinline) void lock() {
				while (InterlockedCompareExchange(reinterpret_cast<volatile long*>(&locker), 1, 0) != 0) {}
			};
			__declspec(noinline) bool try_lock() {
				return InterlockedCompareExchange(reinterpret_cast<volatile long*>(&locker), 1, 0) == 0;
			};
			void unlock() {
				InterlockedDecrement(reinterpret_cast<volatile long*>(&locker));
			};
		};
	private:
		GL::atomic_allocator<dynamic_block, 128, true, true>
			block_alloc;
		GL::parallel_binary_search_tree<dynamic_block, unsigned long long, 10>
			free_tree;
		GL::atomic_vector< buffer_type >
			allocations;
		GL::ticket_dispensor<true>
			allocation_tickets;
		std::function<buffer_type(unsigned long long)>
			alloc_block; // alloc a fresh buffer with length
		std::function<void(buffer_type&)>
			free_parent_block; // release and delete

	public:
		size_t size() const {
			return block_alloc.size();
		};
		__declspec(noinline) dynamic_allocator(
			std::function<buffer_type(unsigned long long)> const& _alloc_block,
			std::function<void(buffer_type&)> const& _free_block
		)
			: block_alloc{}
			, free_tree{}
			, allocations{}
			, allocation_tickets{}
			, alloc_block{ _alloc_block }
			, free_parent_block{ _free_block }
		{};

	public:
		__declspec(noinline) dynamic_block* Alloc(unsigned long long N) {
			dynamic_block* free_block = nullptr;
			while (N > 0) {
				while (!free_block) {
					if (auto tree_locked = free_tree.lock()) {
						if (auto* tree_node = free_tree.NodeFindSmallestLargerEqual_Locked(N, tree_locked); tree_node) {
							while (tree_node) {
								free_block = tree_node->object();
								if (free_block && free_block->try_lock()) {
									if (!free_block->is_available) {
										// cooperatively remove this node
										free_tree.Remove(tree_node, tree_locked);
										free_block->unlock();
										block_alloc.Free(free_block);
										free_block = nullptr;
										break;
									}

									if (free_block->length >= N) {
										if (free_block->is_free) { // we have a winner.
											free_block->is_free = false;
											if (free_block->length > N) {
												// cooperatively remove this node
												free_block->sub_buffer = nullptr;
												this->free_parent_block(this->allocations[free_block->parent_buffer]);
												this->allocations[free_block->parent_buffer] = nullptr;
												allocation_tickets.return_ticket(free_block->parent_buffer);

												free_tree.Remove(tree_node, tree_locked);
												free_block->unlock();
												block_alloc.Free(free_block);
												free_block = nullptr;
												break;
											}
											free_tree.Remove(tree_node, tree_locked); // invalidates the tree
											break;
										}
										else {
											// nothing is wrong with this guy, he simply is being used. He doesn't belong in the tree!
											free_block->unlock();
											free_block = nullptr;
										}
									}
								}
								free_block = nullptr;
								tree_node = free_tree.GetNextLeaf(tree_node, tree_locked);
							}
						}
						else {
							free_block = block_alloc.Alloc();
							if (!free_block) return nullptr;
							free_block->length = N;
							free_block->is_available = true;
							free_block->is_free = false;
							free_block->locker = 1;

							free_block->parent_buffer = allocation_tickets.get_ticket();
							this->allocations.grow_to_at_least(free_block->parent_buffer + 1);
							this->allocations[free_block->parent_buffer] = this->alloc_block(free_block->length);
							if (!this->allocations[free_block->parent_buffer]) {
								allocation_tickets.return_ticket(free_block->parent_buffer);
								block_alloc.Free(free_block);
								return nullptr;
							}
							free_block->sub_buffer = nullptr;
						}
					}
				}

				// if this already has a sub_buffer, we should prefer re-use, even if it is too big.             
				if (!free_block->sub_buffer) {
					// we need to make a sub-buffer, but it's not worth splitting. 
					free_block->sub_buffer = this->allocations[free_block->parent_buffer];
				}

				free_block->unlock();
				return free_block;
			}
			return free_block;
		};
		// must be explicitely free'd before the dynamic_allocator goes out of scope, otherwise memory leak. 
		__declspec(noinline) void Free(dynamic_block* free_block) {
			if (free_block) {
				//free_block->lock();
				free_block->is_free = true;
				free_tree.Add(free_block, free_block->length);
				//free_block->unlock();            
			}
		};

	public:
		__declspec(noinline) ~dynamic_allocator() {
			for (auto& buf : allocations) if (buf) free_parent_block(buf);
		};

		class unique_ptr {
			dynamic_allocator::dynamic_block* data;
			dynamic_allocator* parent;

		public:
			explicit unique_ptr(dynamic_allocator::dynamic_block* d, dynamic_allocator* p) : data{ d }, parent{ p } {};
			unique_ptr() : data{ nullptr }, parent{ nullptr } {};
			unique_ptr(std::nullptr_t) : data{ nullptr }, parent{ nullptr } {};
			unique_ptr(unique_ptr const&) = delete;
			unique_ptr(unique_ptr&& rhs) noexcept : data{ rhs.data }, parent{ rhs.parent } {
				rhs.data = nullptr;
				rhs.parent = nullptr;
			};
			unique_ptr& operator=(unique_ptr const&) = delete;
			__declspec(noinline) unique_ptr& operator=(std::nullptr_t) {
				if (data && parent) { parent->Free(data); }
				data = nullptr;
				parent = nullptr;
				return *this;
			};
			__declspec(noinline) unique_ptr& operator=(unique_ptr&& rhs) noexcept {
				if (data && parent) { parent->Free(data); }
				data = rhs.data;
				parent = rhs.parent;
				rhs.data = nullptr;
				rhs.parent = nullptr;
				return *this;
			};
			operator bool() const {
				return data;
			};

			~unique_ptr() {
				if (data && parent) { parent->Free(data); }
			};

			const dynamic_allocator::dynamic_block* operator->() const {
				return data;
			};
			const dynamic_allocator::dynamic_block* operator->() {
				return data;
			};
			const dynamic_allocator::dynamic_block& operator*() const {
				return *data;
			};
			dynamic_allocator::dynamic_block& operator*() {
				return *data;
			};
			const dynamic_allocator::dynamic_block* get() const {
				return data;
			};
			dynamic_allocator::dynamic_block* get() {
				return data;
			};

		};
		typedef typename std::shared_ptr<dynamic_allocator::dynamic_block> shared_ptr;

		unique_ptr make_unique(unsigned int N) {
			return unique_ptr(this->Alloc(N), this);
		};

		shared_ptr make_shared(unsigned int N) {
			return shared_ptr(this->Alloc(N), [this](dynamic_allocator::dynamic_block* p) {
				this->Free(p);
				});
		};
	};

	// multi-threaded memory manager. Optimized for use in heavy multi-threaded conditions. 
	// Re-using previous allocations is prioritized. If a thread dies before releasing its memory, however, that poses a risk for unreleased memory. 
	// This unreleased memory will be correctly handled on destruction, or if a new thread shows up sharing that old thread's ID (which is also prioritized).
	// This version has a slightly higher memory footprint (5-9%), in exchange for slightly higher performance (5-9%)
	template <typename buffer_type = void*>
	class parallel_dynamic_allocator {
	private:
		GL::thread_object_no_default< dynamic_allocator< buffer_type > >  // collection of allocators organized by thread_id
			allocator;
		std::function<buffer_type(unsigned long long)> // alloc a fresh buffer with length
			alloc_block;
		std::function<void(buffer_type&)> // release and delete
			free_parent_block;

	public:
		using dynamic_block = typename dynamic_allocator< buffer_type >::dynamic_block;
		using unique_ptr = typename dynamic_allocator< buffer_type >::unique_ptr;
		using shared_ptr = typename dynamic_allocator< buffer_type >::shared_ptr;

		size_t size() {
			size_t out = 0;
			allocator.for_each([&out](auto& alloc) {
				out += alloc.size();
			});
			return out;
		};
		parallel_dynamic_allocator(
			std::function<buffer_type(unsigned long long)> const& _alloc_block,
			std::function<void(buffer_type&)> const& _free_block
		)
			: allocator{}
			, alloc_block{ _alloc_block }
			, free_parent_block{ _free_block }
		{};
		~parallel_dynamic_allocator() = default;

		dynamic_block* Alloc(unsigned long long N) {
			if (N > 0) {
				dynamic_block* out = allocator.get_or_init(alloc_block, free_parent_block).Alloc(N);
				out->thread_id = GL::util::get_thread_id();
				return out;
			}
			else {
				return nullptr;
			}
		};
		void Free(dynamic_block* free_block) {
			if (free_block) {
				allocator[free_block->thread_id].Free(free_block);
			}
		};

	public:
		unique_ptr make_unique(unsigned int N) {
			if (auto* p = this->Alloc(N)) {
				return unique_ptr(p, &allocator.get_or_init(alloc_block, free_parent_block));
			}
			return nullptr;
		};
		shared_ptr make_shared(unsigned int N) {
			if (N > 0) {
				return shared_ptr(this->Alloc(N), [this](dynamic_block* p) {
					this->Free(p);
			    });
			}
			else {
				return nullptr;
			}
		};

	};
};