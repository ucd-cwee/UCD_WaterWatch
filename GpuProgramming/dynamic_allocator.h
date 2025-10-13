#pragma once
#include <memory>
#include <set>
// #include <corecrt_malloc.h>

static void* Mem_Alloc16(const size_t& size) {
	if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16);
};
static void  Mem_Free16(void* ptr) {
	if (ptr) ::_aligned_free(ptr);
};
static void* Mem_ClearedAlloc(const size_t& size) {
	void* mem = Mem_Alloc16(size);
	::memset(mem, 0, size);
	return mem;
};
static void  Mem_Free(void* ptr) { Mem_Free16(ptr); };
static void* Mem_Alloc(const size_t& size) { return Mem_Alloc16(size); };

/*
================================================
cweeBlockAlloc is a block-based allocator for fixed-size objects.
All objects are properly constructed and destructed.
================================================
*/
#define BLOCK_ALLOC_ALIGNMENT 16
#define CONST_MAX( x, y ) ( (x) > (y) ? (x) : (y) )

template<class _type_, int _blockSize_>
class cweeBlockAlloc {
public:
	cweeBlockAlloc(bool clear = true) : // = false
		blocks(NULL),
		free(NULL),
		total(0),
		active(0),
		allowAllocs(true),
		clearAllocs(clear)
	{};
	cweeBlockAlloc(int toReserve) :
		blocks(NULL),
		free(NULL),
		total(0),
		active(0),
		allowAllocs(true),
		clearAllocs(true)
	{
		Reserve(toReserve);
	};
	~cweeBlockAlloc() {
		Shutdown();
	};

	// returns total size of allocated memory
	size_t				Allocated() const { return total * sizeof(_type_); }

	// returns total size of allocated memory including size of (*this)
	size_t				Size() const { return sizeof(*this) + Allocated(); }

	void				Shutdown() {
		while (blocks != NULL) {
			cweeBlock* block = blocks;
			blocks = blocks->next;
			Mem_Free(block);
		}
		blocks = NULL;
		free = NULL;
		total = active = 0;
	};
	__forceinline void			SetFixedBlocks(long long numBlocks) {
		long long currentNumBlocks = 0;
		for (cweeBlock* block = blocks; block != NULL; block = block->next) {
			currentNumBlocks++;
		}
		for (long long i = currentNumBlocks; i < numBlocks; i++) {
			AllocNewBlock();
		}
		allowAllocs = false;
	};
	__forceinline void			FreeEmptyBlocks() {
		// first count how many free elements are in each block and build up a free chain per block
		for (cweeBlock* block = blocks; block != NULL; block = block->next) {
			block->free = NULL;
			block->freeCount = 0;
		}
		for (element_t* element = free; element != NULL; ) {
			element_t* next = element->next;
			for (cweeBlock* block = blocks; block != NULL; block = block->next) {
				if (element >= block->elements && element < block->elements + _blockSize_) {
					element->next = block->free;
					block->free = element;
					block->freeCount++;
					break;
				}
			}
			element = next;
		}
		// now free all blocks whose free count == _blockSize_
		cweeBlock* prevBlock = NULL;
		for (cweeBlock* block = blocks; block != NULL; ) {
			cweeBlock* next = block->next;
			if (block->freeCount == _blockSize_) {
				if (prevBlock == NULL) {
					blocks = block->next;
				}
				else {
					prevBlock->next = block->next;
				}
				Mem_Free(block);
				total -= _blockSize_;
			}
			else {
				prevBlock = block;
			}
			block = next;
		}
		// now rebuild the free chain
		free = NULL;
		for (cweeBlock* block = blocks; block != NULL; block = block->next) {
			for (element_t* element = block->free; element != NULL; ) {
				element_t* next = element->next;
				element->next = free;
				free = element;
				element = next;
			}
		}
	};

	static constexpr bool isPod() { return std::is_pod<_type_>::value; };
	_type_* Alloc() {
		if (free == NULL) {
			if (!allowAllocs) {
				return NULL;
			}
			AllocNewBlock();
		}

		active++;
		element_t* element = free;
		free = free->next;
		element->next = NULL;

		_type_* t = (_type_*)element->buffer;
		if constexpr (isPod()) {
			memset(t, 0, sizeof(_type_));
		}
		else {
			if (clearAllocs) {
				memset(t, 0, sizeof(_type_));
			}
			new (t) _type_;
		}

		return t;
	};

	void				Free(_type_* element) {
		if (element == nullptr) {
			return;
		}

		if constexpr (!isPod()) {
			element->~_type_();
		}

		element_t* t = (element_t*)(element);
		t->next = free;
		free = t;
		active--;
	};
	__forceinline void			Reserve(long long num) {
		if (total < num) {
			std::vector< _type_* > arr; arr.reserve(2 * (num - total));
			while (total < num) {
				arr.push_back(Alloc());
			}
			for (_type_* p : arr) {
				Free(p);
			}
		}
	};
	long long			GetTotalCount() const { return total; }
	long long			GetAllocCount() const { return active; }
	long long			GetFreeCount() const { return total - active; }

private:
	union element_t {
		_type_* data;
		element_t* next;
		::byte			buffer[(CONST_MAX(sizeof(_type_), sizeof(element_t*)) + (BLOCK_ALLOC_ALIGNMENT - 1)) & ~(BLOCK_ALLOC_ALIGNMENT - 1)];
	};

	class cweeBlock {
	public:
		element_t		elements[_blockSize_];
		cweeBlock* next;
		element_t* free;		// list with free elements in this block (temp used only by FreeEmptyBlocks)
		long long		freeCount;	// number of free elements in this block (temp used only by FreeEmptyBlocks)
	};

	cweeBlock* blocks;
	element_t* free;
	long long			total;
	long long			active;
	bool				allowAllocs;
	bool				clearAllocs;

	void			AllocNewBlock() {
		cweeBlock* block = (cweeBlock*)Mem_Alloc((size_t)(sizeof(cweeBlock)));
		block->next = blocks;
		blocks = block;
		for (int i = 0; i < _blockSize_; i++) {
			block->elements[i].next = free;
			free = &block->elements[i];
		}
		total += _blockSize_;
	};
};

template<class _type_, size_t BlockSize = 128>
class cweeAlloc {
private:
	static constexpr bool isPod() { return std::is_pod<_type_>::value; };
public:
	cweeAlloc() : ptrs(), alloc() {};
	cweeAlloc(int toReserve) : ptrs(), alloc(toReserve) {};
	~cweeAlloc() { Clear(); };

	_type_* Alloc() {
		decltype(auto) p = alloc.Alloc();
		if constexpr (!isPod()) {
			ptrs.insert(p);
		}
		return p;
	};
	void	Free(_type_* element) {
		if constexpr (!isPod()) {
			ptrs.erase(element);
		}
		alloc.Free(element);
	};
	void	Clean() {
		alloc.FreeEmptyBlocks();
	};
	long long	GetTotalCount() const {
		long long out;
		out = alloc.GetTotalCount();
		return out;
	};
	long long	GetAllocCount() const {
		long long out;
		if constexpr (!isPod()) {
			out = ptrs.size();
		}
		else {
			out = alloc.GetAllocCount();
		}
		return out;
	};
	void	Clear() {
		if constexpr (!isPod()) {
			for (auto& x : ptrs) {
				if (x != nullptr) {
					alloc.Free(x);
				}
			}
			ptrs.clear();
		}
		else {
			alloc.Shutdown();
			alloc.Free(alloc.Alloc());
		}
	};
	void	Reserve(long long n) {
		alloc.Reserve(n);
	};

private:
	std::set<_type_*> ptrs;
	cweeBlockAlloc<_type_, BlockSize> alloc;
};

#include "BTree.h"

template<class type, int additional_buffer = 0>
class cweeDynamicBlock {
public:
	type* 
		GetMemory() const { return (type*)(((::byte*)this) + sizeof(cweeDynamicBlock<type, additional_buffer>)); }
	int								
		GetSize() const { return abs(size); }
	void							
		SetSize(int s, bool isBaseBlock) { size = isBaseBlock ? -s : s; }
	bool							
		IsBaseBlock() const { return (size < 0); }
	
	int								
		size = 0;					// size in bytes of the block
	cweeDynamicBlock<type, additional_buffer>*
		prev = NULL;					// previous memory block
	cweeDynamicBlock<type, additional_buffer>*
		next = NULL;					// next memory block
	cweeBTreeNode<cweeDynamicBlock<type, additional_buffer>, int>*
		node = NULL;			// node in the B-Tree with free blocks
	char
		padding[additional_buffer];
};

template<class type, int baseBlockSize, int minBlockSize, int additional_buffer = 0>
class cweeDynamicBlockAlloc {
public:
	cweeDynamicBlockAlloc() {
		Clear(); 
		Init();
	};
	~cweeDynamicBlockAlloc() { 
		Shutdown(); 
	};

	void							Init() {
		freeTree.Init();
		Free(Alloc(2));
	};
	void							Shutdown() {
		Free(Alloc(2));

		cweeDynamicBlock<type, additional_buffer>* block;

		for (block = firstBlock; block != NULL; block = block->next) {
			if (block->node == NULL) {
				FreeInternal(block);
			}
		}

		for (block = firstBlock; block != NULL; block = firstBlock) {
			firstBlock = block->next;
			Mem_Free16(block);
		}

		freeTree.Shutdown();

		Clear();
	};
	void							SetFixedBlocks(int numBlocks) {
		cweeDynamicBlock<type, additional_buffer>* block;

		for (int i = numBaseBlocks; i < numBlocks; i++) {
			block = (cweeDynamicBlock<type, additional_buffer>*) Mem_Alloc16((size_t)baseBlockSize);
			block->SetSize(baseBlockSize - (int)sizeof(cweeDynamicBlock<type, additional_buffer>), true);
			block->next = NULL;
			block->prev = lastBlock;
			if (lastBlock) {
				lastBlock->next = block;
			}
			else {
				firstBlock = block;
			}
			lastBlock = block;
			block->node = NULL;

			FreeInternal(block);

			numBaseBlocks++;
			baseBlockMemory += baseBlockSize;
		}

		allowAllocs = false;
	};
	void							FreeEmptyBaseBlocks() {
		cweeDynamicBlock<type, additional_buffer>
			*block, 
			*next;

		for (block = firstBlock; block != NULL; block = next) {
			next = block->next;

			if (block->IsBaseBlock() && block->node != NULL && (next == NULL || next->IsBaseBlock())) {
				UnlinkFreeInternal(block);
				if (block->prev) {
					block->prev->next = block->next;
				}
				else {
					firstBlock = block->next;
				}
				if (block->next) {
					block->next->prev = block->prev;
				}
				else {
					lastBlock = block->prev;
				}
				numBaseBlocks--;
				baseBlockMemory -= block->GetSize() + (int)sizeof(cweeDynamicBlock<type, additional_buffer>);
				Mem_Free16(block);
			}
		}

	};

	type* Alloc(const int num, bool clearMemory = false) {
		if (num <= 0) {
			return NULL;
		}
		else {
			cweeDynamicBlock<type, additional_buffer>* block;

			block = AllocInternal(num);
			if (block == NULL) {
				return NULL;
			}
			block = ResizeInternal(block, num);
			if (block == NULL) {
				return NULL;
			}

			numUsedBlocks++;
			usedBlockMemory += block->GetSize();

			type* ptr = block->GetMemory();
			if constexpr (std::is_pod<type>::value) {
				if (clearMemory) {
					::memset((void*)ptr, 0, sizeof(type) * num);
				}
			}
			else {
				for (int i = 0; i < num; ++i) {
					new (ptr + i) type();
				}
			}

			return ptr;
		}
	};
	type* Resize(type* ptr, const int num, bool clearMemory = false) {
		if (ptr == NULL) {
			return Alloc(num, clearMemory);
		}
		if (num <= 0) {
			Free(ptr);
			return NULL;
		}
		cweeDynamicBlock<type, additional_buffer>* block = (cweeDynamicBlock<type, additional_buffer>*) (((::byte*)ptr) - (int)sizeof(cweeDynamicBlock<type, additional_buffer>));
		usedBlockMemory -= block->GetSize();
		block = ResizeInternal(block, num);
		if (block == NULL) {
			return NULL;
		}
		usedBlockMemory += block->GetSize();

		type* p = block->GetMemory();
		if (clearMemory && std::is_pod<type>::value) {
			::memset((void*)p, 0, sizeof(type) * num);
		}
		return p;
	};
	void  Free(type* ptr) {
		if (!ptr) { return; }
		cweeDynamicBlock<type, additional_buffer>* block = (cweeDynamicBlock<type, additional_buffer>*) (((::byte*)ptr) - (int)sizeof(cweeDynamicBlock<type, additional_buffer>));
		if constexpr (!std::is_pod<type>::value) {
			for (int i = 0; i < block->size; ++i) {
				(ptr + i)->~type();
			}
		}
		numUsedBlocks--;
		usedBlockMemory -= block->GetSize();
		FreeInternal(block);
	};

	int								GetNumBaseBlocks() const { return numBaseBlocks; }
	int								GetBaseBlockMemory() const { return baseBlockMemory; }
	int								GetNumUsedBlocks() const { return numUsedBlocks; }
	int								GetUsedBlockMemory() const { return usedBlockMemory; }
	int								GetNumFreeBlocks() const { return numFreeBlocks; }
	int								GetFreeBlockMemory() const { return freeBlockMemory; }
	int								GetNumEmptyBaseBlocks() const {
		int numEmptyBaseBlocks;
		cweeDynamicBlock<type, additional_buffer>* block;

		numEmptyBaseBlocks = 0;
		for (block = firstBlock; block != NULL; block = block->next) {
			if (block->IsBaseBlock() && block->node != NULL && (block->next == NULL || block->next->IsBaseBlock())) {
				numEmptyBaseBlocks++;
			}
		}
		return numEmptyBaseBlocks;
	};

private:
	cweeDynamicBlock<type, additional_buffer>* firstBlock;				// first block in list in order of increasing address
	cweeDynamicBlock<type, additional_buffer>* lastBlock;				// last block in list in order of increasing address
	cweeBTree<cweeDynamicBlock<type, additional_buffer>, int, 4>freeTree;			// B-Tree with free memory blocks
	bool							allowAllocs = true;			// allow base block allocations
	int								numBaseBlocks = 0;			// number of base blocks
	int								baseBlockMemory = 0;		// total memory in base blocks
	int								numUsedBlocks = 0;			// number of used blocks
	int								usedBlockMemory = 0;		// total memory in used blocks
	int								numFreeBlocks = 0;			// number of free blocks
	int								freeBlockMemory = 0;		// total memory in free blocks

	void							Clear() {
		firstBlock = NULL;
		lastBlock = NULL;
		allowAllocs = true;
		numBaseBlocks = 0;
		baseBlockMemory = 0;
		numUsedBlocks = 0;
		usedBlockMemory = 0;
		numFreeBlocks = 0;
		freeBlockMemory = 0;
	};
	cweeDynamicBlock<type, additional_buffer>* AllocInternal(const int num) {
		cweeDynamicBlock<type, additional_buffer>* block;
		int alignedBytes = (num * sizeof(type) + 15) & ~15;

		block = freeTree.FindSmallestLargerEqual(alignedBytes);
		if (block && (block != NULL) && (block != nullptr)) {
			UnlinkFreeInternal(block);
		}
		else if (allowAllocs) {
			int allocSize = CONST_MAX(baseBlockSize, alignedBytes + (int)sizeof(cweeDynamicBlock<type, additional_buffer>));
			block = (cweeDynamicBlock<type, additional_buffer>*) Mem_Alloc16((size_t)allocSize);
			block->SetSize(allocSize - (int)sizeof(cweeDynamicBlock<type, additional_buffer>), true);
			block->next = NULL;
			block->prev = lastBlock;
			if (lastBlock) {
				lastBlock->next = block;
			}
			else {
				firstBlock = block;
			}
			lastBlock = block;
			block->node = NULL;

			numBaseBlocks++;
			baseBlockMemory += allocSize;
		}

		return block;
	};
	cweeDynamicBlock<type, additional_buffer>* ResizeInternal(cweeDynamicBlock<type, additional_buffer>* block, const int num) {
		int alignedBytes = (num * sizeof(type) + 15) & ~15;
		// if the new size is larger
		if (alignedBytes > block->GetSize()) {

			cweeDynamicBlock<type, additional_buffer>* nextBlock = block->next;

			// try to annexate the next block if it's free
			if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
				block->GetSize() + (int)sizeof(cweeDynamicBlock<type, additional_buffer>) + nextBlock->GetSize() >= alignedBytes) {

				UnlinkFreeInternal(nextBlock);
				block->SetSize(block->GetSize() + (int)sizeof(cweeDynamicBlock<type, additional_buffer>) + nextBlock->GetSize(), block->IsBaseBlock());
				block->next = nextBlock->next;
				if (nextBlock->next) {
					nextBlock->next->prev = block;
				}
				else {
					lastBlock = block;
				}
			}
			else {
				// allocate a new block and copy
				cweeDynamicBlock<type, additional_buffer>* oldBlock = block;
				block = AllocInternal(num);
				if (block == NULL) {
					return NULL;
				}
				memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
				FreeInternal(oldBlock);
			}
		}

		// if the unused space at the end of this block is large enough to hold a block with at least one element
		if (block->GetSize() - alignedBytes - (int)sizeof(cweeDynamicBlock<type, additional_buffer>) < CONST_MAX(minBlockSize, (int)sizeof(type))) {
			return block;
		}

		cweeDynamicBlock<type, additional_buffer>* newBlock;

		newBlock = (cweeDynamicBlock<type, additional_buffer>*) (((::byte*)block) + (int)sizeof(cweeDynamicBlock<type, additional_buffer>) + alignedBytes);
		try {
			newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(cweeDynamicBlock<type, additional_buffer>), false);
		}
		catch (...) {}
		newBlock->next = block->next;
		newBlock->prev = block;
		if (newBlock->next != NULL) {
			newBlock->next->prev = newBlock;
		}
		else {
			lastBlock = newBlock;
		}
		newBlock->node = NULL;
		block->next = newBlock;
		block->SetSize(alignedBytes, block->IsBaseBlock());

		FreeInternal(newBlock);

		return block;
	};
	void							FreeInternal(cweeDynamicBlock<type, additional_buffer>* block) {
		// try to merge with a next free block
		cweeDynamicBlock<type, additional_buffer>* nextBlock = block->next;
		if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL) {
			UnlinkFreeInternal(nextBlock);
			block->SetSize(block->GetSize() + (int)sizeof(cweeDynamicBlock<type, additional_buffer>) + nextBlock->GetSize(), block->IsBaseBlock());
			block->next = nextBlock->next;
			if (nextBlock->next) {
				nextBlock->next->prev = block;
			}
			else {
				lastBlock = block;
			}
		}

		// try to merge with a previous free block
		cweeDynamicBlock<type, additional_buffer>* prevBlock = block->prev;
		//if (prevBlock && !block->IsBaseBlock() && prevBlock->node != NULL) {
		if (prevBlock && !prevBlock->IsBaseBlock() && prevBlock->node != NULL) {
			UnlinkFreeInternal(prevBlock);
			prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(cweeDynamicBlock<type, additional_buffer>) + block->GetSize(), prevBlock->IsBaseBlock());
			prevBlock->next = block->next;
			if (block->next) {
				block->next->prev = prevBlock;
			}
			else {
				lastBlock = prevBlock;
			}
			LinkFreeInternal(prevBlock);
		}
		else {
			LinkFreeInternal(block);
		}
	};
	void							LinkFreeInternal(cweeDynamicBlock<type, additional_buffer>* block) {
		block->node = freeTree.Add(block, block->GetSize());
		numFreeBlocks++;
		freeBlockMemory += block->GetSize();
	};
	void							UnlinkFreeInternal(cweeDynamicBlock<type, additional_buffer>* block) {
		freeTree.Remove(block->node);
		block->node = NULL;
		numFreeBlocks--;
		freeBlockMemory -= block->GetSize();
	};
};
