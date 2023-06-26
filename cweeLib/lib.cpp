
/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#include "precompiled.h"
#pragma hdrstop

/*
===============================================================================

	cweeLib

===============================================================================
*/
cweeSys* cweeLib::sys = NULL;
void* cweeLib::edms_epanet = NULL;
void* cweeLib::edms_rtdb = NULL;

/*
================
cweeLib::Init
================
*/
void cweeLib::Init(void) {
	// initialize little/big endian conversion
	Swap_Init();

	// init string memory allocator
	cweeStr::InitMemory();

	// initialize SIMD implementation
	cweeSIMD::Init();

	// initialize math
	cweeMath::Init();

	// initialize the FileSystem
	fileSystem->Init();

#ifdef debugUwpInstallation
	fileSystem->appendLog(fileSystem->createFilePath(fileSystem->getDataFolder(), "debugUwpInstallation",TXT), " - Initiated File System - ");
#endif

	// initialize the major job threads
	parallelProcessor->Init();
	parallelProcessor->AllocJobList(SIM_thread, (jobListPriority_t)JOBLIST_PRIORITY_HIGH,			20048 * 1, 100);
	parallelProcessor->AllocJobList(SIM_thread, (jobListPriority_t)JOBLIST_PRIORITY_MEDIUM,			20048 * 1, 100);
	parallelProcessor->AllocJobList(SIM_thread, (jobListPriority_t)JOBLIST_PRIORITY_LOW,			20048 * 1, 100);
	parallelProcessor->AllocJobList(OPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_HIGH,			20048 * 5, 100);
	parallelProcessor->AllocJobList(OPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_MEDIUM,			20048 * 5, 100);
	parallelProcessor->AllocJobList(OPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_LOW,			20048 * 5, 100);
	parallelProcessor->AllocJobList(IO_thread, (jobListPriority_t)JOBLIST_PRIORITY_HIGH,			20048 * 5, 100);
	parallelProcessor->AllocJobList(IO_thread, (jobListPriority_t)JOBLIST_PRIORITY_MEDIUM,			20048 * 5, 100);
	parallelProcessor->AllocJobList(IO_thread, (jobListPriority_t)JOBLIST_PRIORITY_LOW,				20048 * 5, 100);
	parallelProcessor->AllocJobList(PRIORITY_thread, (jobListPriority_t)JOBLIST_PRIORITY_HIGH,		20048 * 1, 100);
	parallelProcessor->AllocJobList(PRIORITY_thread, (jobListPriority_t)JOBLIST_PRIORITY_MEDIUM,	20048 * 1, 100);
	parallelProcessor->AllocJobList(PRIORITY_thread, (jobListPriority_t)JOBLIST_PRIORITY_LOW,		20048 * 1, 100);
	//parallelProcessor->AllocJobList(SCRIPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_HIGH,		20048 * 1, 100);
	//parallelProcessor->AllocJobList(SCRIPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_MEDIUM,		20048 * 1, 100);
	//parallelProcessor->AllocJobList(SCRIPT_thread, (jobListPriority_t)JOBLIST_PRIORITY_LOW,			20048 * 1, 100);

#ifdef debugUwpInstallation
	fileSystem->appendLog(fileSystem->createFilePath(fileSystem->getDataFolder(), "debugUwpInstallation", TXT), " - Initiated parallelProcessor System - ");
#endif

	// INITIATE THE RUNTIME SCRIPTING ENGINE
	scripting->Init();
}

/*
================
cweeLib::ShutDown
================
*/
void cweeLib::ShutDown(void) {

	// RELEASE THE RUNTIME SCRIPTING ENGINE
	scripting->ShutDown();

	// FileSystem does not require a shutdown

	// shutdown the parallel threads
	cweeMultithreading::WAIT();
	parallelProcessor->Shutdown();

	// math system does not require a shutdown

	// shut down the SIMD engine
	cweeSIMD::Shutdown();

	// shut down the string memory allocator
	cweeStr::ShutdownMemory();
}




std::string															cweeLib::Mem_DefineTag(int tag) {
	// manual definitions. 
	std::string out;

	switch (tag) {
	case 0: {
		out = "UNSET";
		break; }
	case 1: {
		out = "DEBUG";
		break; }
	case 2: {
		out = "GENERAL NEW";
		break; }
	case 3: {
		out = "BLOCK ALLOC";
		break; }
	case 4: {
		out = "LIST";
		break; }
	case 5: {
		out = "TEMP";
		break; }
	case 6: {
		out = "MATH";
		break; }
	case 7: {
		out = "FX";
		break; }
	case 8: {
		out = "STRING";
		break; }
	case 9: {
		out = "JOBLIST";
		break; }
	case 10: {
		out = "RUNTIME_DB";
		break; }
	}
	return out;
};

std::vector< std::string >												cweeLib::Mem_GetTagsUsed() {
	std::vector< std::string > out;

//	#ifdef			FORCE_MEM_TRACKING

	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);
	std::ifstream file("memoryMap.txt"); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);	
		cweeStr line = get.c_str();
		cweeParser obj (line, ",", true);
		get = Mem_DefineTag((int)obj[0]);
		bool unique = true;
		for (auto& x : out) {
			if (x == get) unique = false;
		}
		if (unique)	out.push_back(get);
	}
	file.close();

//	#endif
	return out;
}

uint64																cweeLib::Mem_GetNumAllocations() {
	uint64 out = 0;

	// pre-read to determine size of file to pre-load memory
	std::string get; std::string container;
	std::ifstream file("memoryMap.txt"); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);
		container += get;
		container += "\n";
	}
	file.close();

	cweeStr temp = container.c_str();
	cweeParser obj (temp, "\n", true);
	for (auto& x : obj) {
		cweeParser obj2 (x, ",", true);
		if ((int)obj2[4] == 1) out++;
	}	

	return out;
}

uint64																cweeLib::Mem_GetNumAllocations(int tag) {
	uint64 out = 0;
	
//#ifdef			FORCE_MEM_TRACKING
	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);
	std::ifstream file("memoryMap.txt"); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);
		cweeStr line = get.c_str();
		cweeParser obj (line, ",", true);
		if (tag == (int)obj[0]) {
			if ((int)obj[4] == 1) out++;
		}
	}
	file.close();
//#endif

	return out;
}

uint64																cweeLib::Mem_GetNumActiveAllocations() {
	uint64 out = 0;
//#ifdef			FORCE_MEM_TRACKING
	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);
	std::ifstream file("memoryMap.txt"); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);
		cweeStr line = get.c_str();
		cweeParser obj(line, ",", true);
		if ((int)obj[4] == 1) out++; else out--;
	}
	file.close();
//#endif
	return out;
}

uint64																cweeLib::Mem_GetSumAllocations() {
	uint64 out = 0;
//#ifdef			FORCE_MEM_TRACKING
	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);
	std::ifstream file("memoryMap.txt"); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);
		cweeStr line = get.c_str();
		cweeParser obj(line, ",", true);
		if ((int)obj[4] == 1) out+=(int)obj[1]; else out-=(int)obj[1];
	}
	file.close();
//#endif
	return out;
}

std::vector<std::tuple<int, int, uintptr_t, uintptr_t, bool>>		cweeLib::Mem_GetHistory() {
	std::vector<std::tuple<int, int, uintptr_t, uintptr_t, bool>> out;
	//#ifdef			FORCE_MEM_TRACKING
	//	((std::mutex*)cweeLib::Mem_Mutex)->lock();
	//	auto copy = *((std::vector<std::tuple<int, int, uintptr_t, uintptr_t, bool>>*)cweeLib::Mem_Memory);
	//	((std::mutex*)cweeLib::Mem_Mutex)->unlock();
	//	return copy;
	//#endif
	return out;
}



















/*
===============================================================================

	Byte order functions

===============================================================================
*/

// can't just use function pointers, or dll linkage can mess up
static short	(*_BigShort)(short l);
static short	(*_LittleShort)(short l);
static int		(*_BigLong)(int l);
static int		(*_LittleLong)(int l);
static float	(*_BigFloat)(float l);
static float	(*_LittleFloat)(float l);
static void		(*_BigRevBytes)(void* bp, int elsize, int elcount);
static void		(*_LittleRevBytes)(void* bp, int elsize, int elcount);
static void     (*_LittleBitField)(void* bp, int elsize);
static void		(*_SixtetsForInt)(byte* out, int src);
static int		(*_IntForSixtets)(byte* in);

short	BigShort(short l) { return _BigShort(l); }
short	LittleShort(short l) { return _LittleShort(l); }
int		BigLong(int l) { return _BigLong(l); }
int		LittleLong(int l) { return _LittleLong(l); }
float	BigFloat(float l) { return _BigFloat(l); }
float	LittleFloat(float l) { return _LittleFloat(l); }
void	BigRevBytes(void* bp, int elsize, int elcount) { _BigRevBytes(bp, elsize, elcount); }
void	LittleRevBytes(void* bp, int elsize, int elcount) { _LittleRevBytes(bp, elsize, elcount); }
void	LittleBitField(void* bp, int elsize) { _LittleBitField(bp, elsize); }

void	SixtetsForInt(byte* out, int src) { _SixtetsForInt(out, src); }
int		IntForSixtets(byte* in) { return _IntForSixtets(in); }

/*
================
ShortSwap
================
*/
short ShortSwap(short l) {
	byte    b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

/*
================
ShortNoSwap
================
*/
short ShortNoSwap(short l) {
	return l;
}

/*
================
LongSwap
================
*/
int LongSwap(int l) {
	byte    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

/*
================
LongNoSwap
================
*/
int	LongNoSwap(int l) {
	return l;
}

/*
================
FloatSwap
================
*/
float FloatSwap(float f) {
	union {
		float	f;
		byte	b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

/*
================
FloatNoSwap
================
*/
float FloatNoSwap(float f) {
	return f;
}

/*
=====================================================================
RevBytesSwap

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.
===================================================================== */
void RevBytesSwap(void* bp, int elsize, int elcount) {
	//register unsigned char* p, * q;
	unsigned char* p, * q;

	p = (unsigned char*)bp;

	if (elsize == 2) {
		q = p + 1;
		while (elcount--) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			p += 2;
			q += 2;
		}
		return;
	}

	while (elcount--) {
		q = p + elsize - 1;
		while (p < q) {
			*p ^= *q;
			*q ^= *p;
			*p ^= *q;
			++p;
			--q;
		}
		p += elsize >> 1;
	}
}

/*
 =====================================================================
 RevBytesSwap

 Reverses byte order in place, then reverses bits in those bytes

 INPUTS
 bp       bitfield structure to reverse
 elsize   size of the underlying data type

 RESULTS
 Reverses the bitfield of size elsize.
 ===================================================================== */
void RevBitFieldSwap(void* bp, int elsize) {
	int i;
	unsigned char* p, t, v;

	LittleRevBytes(bp, elsize, 1);

	p = (unsigned char*)bp;
	while (elsize--) {
		v = *p;
		t = 0;
		for (i = 7; i; i--) {
			t <<= 1;
			v >>= 1;
			t |= v & 1;
		}
		*p++ = t;
	}
}

/*
================
RevBytesNoSwap
================
*/
void RevBytesNoSwap(void* bp, int elsize, int elcount) {
	return;
}

/*
 ================
 RevBytesNoSwap
 ================
 */
void RevBitFieldNoSwap(void* bp, int elsize) {
	return;
}

/*
================
SixtetsForIntLittle
================
*/
void SixtetsForIntLittle(byte* out, int src) {
	byte* b = (byte*)&src;
	out[0] = (b[0] & 0xfc) >> 2;
	out[1] = ((b[0] & 0x3) << 4) + ((b[1] & 0xf0) >> 4);
	out[2] = ((b[1] & 0xf) << 2) + ((b[2] & 0xc0) >> 6);
	out[3] = b[2] & 0x3f;
}

/*
================
SixtetsForIntBig
TTimo: untested - that's the version from initial base64 encode
================
*/
void SixtetsForIntBig(byte* out, int src) {
	for (int i = 0; i < 4; i++) {
		out[i] = src & 0x3f;
		src >>= 6;
	}
}

/*
================
IntForSixtetsLittle
================
*/
int IntForSixtetsLittle(byte* in) {
	int ret = 0;
	byte* b = (byte*)&ret;
	b[0] |= in[0] << 2;
	b[0] |= (in[1] & 0x30) >> 4;
	b[1] |= (in[1] & 0xf) << 4;
	b[1] |= (in[2] & 0x3c) >> 2;
	b[2] |= (in[2] & 0x3) << 6;
	b[2] |= in[3];
	return ret;
}

/*
================
IntForSixtetsBig
TTimo: untested - that's the version from initial base64 decode
================
*/
int IntForSixtetsBig(byte* in) {
	int ret = 0;
	ret |= in[0];
	ret |= in[1] << 6;
	ret |= in[2] << 2 * 6;
	ret |= in[3] << 3 * 6;
	return ret;
}

/*
================
Swap_Init
================
*/
void Swap_Init(void) {
	byte	swaptest[2] = { 1,0 };

	// set the byte swapping variables in a portable manner	
	if (*(short*)swaptest == 1) {
		// little endian ex: x86
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
		_BigRevBytes = RevBytesSwap;
		_LittleRevBytes = RevBytesNoSwap;
		_LittleBitField = RevBitFieldNoSwap;
		_SixtetsForInt = SixtetsForIntLittle;
		_IntForSixtets = IntForSixtetsLittle;
	}
	else {
		// big endian ex: ppc
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
		_BigRevBytes = RevBytesNoSwap;
		_LittleRevBytes = RevBytesSwap;
		_LittleBitField = RevBitFieldSwap;
		_SixtetsForInt = SixtetsForIntBig;
		_IntForSixtets = IntForSixtetsBig;
	}
}

/*
==========
Swap_IsBigEndian
==========
*/
bool Swap_IsBigEndian(void) {
	byte	swaptest[2] = { 1,0 };
	return *(short*)swaptest != 1;
}
