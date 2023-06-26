
#include "precompiled.h"
#pragma hdrstop






#define CURL_STATICLIB
#include "../EDMS_DLL_Codebase/include/curl_curl.h"

#pragma comment(lib, "Iphlpapi.lib")

#include "zeroMQ/zmq_addon.hpp"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"Wldap32.lib")
#pragma comment(lib,"Normaliz.lib")

#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define useCurveSecurity

#define MAX_ZIPPED_FILE_NAME	2048
#define FILE_HASH_SIZE			1024
// #define useFasterMkTime
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

//class cweeTM {
//public:
//	cweeTM() { 
//		struct tm out1 {};
//		time = out1; };
//	cweeTM(const tm& t) {
//		time = t;
//	};
//	~cweeTM() {};
//
//	explicit operator tm() {
//		return time;
//	};
//	explicit operator const tm() const {
//		return time;
//	};
//
//	cweeTM& operator=(tm const& other) {
//		time = other;
//		return *this;
//	};
//	cweeTM& operator=(cweeTM const& other) {
//		time = other.time;
//		return *this;
//	};
//
//	bool operator==(cweeTM const& other) const {
//		if (other.time.tm_year == time.tm_year) 
//			if (other.time.tm_mon == time.tm_mon) 
//				if (other.time.tm_mday == time.tm_mday) 
//					if (other.time.tm_hour == time.tm_hour) 
//						if (other.time.tm_min == time.tm_min) 
//							if (other.time.tm_sec == time.tm_sec) 
//								return true;											
//		return false;
//	};
//	bool operator!=(cweeTM const& other) const {
//		return !operator==(other);
//	};
//
//	bool operator<(cweeTM const& other) const {
//		if (other.time.tm_year >= time.tm_year)
//			if (other.time.tm_mon >= time.tm_mon)
//				if (other.time.tm_mday >= time.tm_mday)
//					if (other.time.tm_hour >= time.tm_hour)
//						if (other.time.tm_min >= time.tm_min)
//							if (other.time.tm_sec >= time.tm_sec)
//								return true;
//		return false;
//	};
//	bool operator>(cweeTM const& other) const {
//		return other < *this;
//	};
//	bool operator>=(cweeTM const& other)const {
//		return !operator<(other);
//	};
//	bool operator<=(cweeTM const& other)const {
//		return !operator>(other);
//	};
//
//	struct tm time {};
//};

size_t fileSys_write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written = std::fwrite(ptr, size, nmemb, stream);
	return written;
};

typedef struct fileInPack_s {
	cweeStr					name;
	unsigned long			pos;
	struct fileInPack_s*	next;
} fileInPack_t;

typedef enum {
	BINARY_UNKNOWN = 0,
	BINARY_YES,
	BARY_NO
} binaryStatus_t;

class directory_t {
public:
	cweeStr			path;
	cweeStr			dataDir;
};

class cweeSocket {
public:
	cweeStr			Name;
	cweeStr			address;
	int				port = 0;
	zmq::socket_t	socket;
	bool			awaitingReply = false;
};

class cweeRouter {
public:
	~cweeRouter() {
		proxyThread.detach();
	};

	cweeStr			Name;
	int				frontend_port = 0;
	cweeStr			backend_name = "";
	zmq::socket_t	frontend_socket;
	zmq::socket_t	backend_socket;
	zmq::socket_t	backend_worker_socket;

	cweeStr			frontend_socket_secretKey;
	cweeStr			backend_socket_secretKey;
	cweeStr			backend_worker_socket_secretKey;

	void		StartRouterProxy() {
		zmq::proxy(static_cast<void*>(frontend_socket),
			static_cast<void*>(backend_socket),
			nullptr);
	};

	std::thread		proxyThread;
};

class cweeRouterRequest {
public:
	cweeRouterRequest() : mut(new cweeSysMutex()) {};

	bool TryLock() {
		return mut->Lock(false);
	};
	void Lock() {
		mut->Lock();
	};
	void Unlock() {
		mut->Unlock();
	};
	
	cweeStr 		routerPort;
	cweeStr 		request;
	cweeStr			reply;
	zmq::message_t	requester_id;
	int				request_index;

	bool			started = false;
	bool			completed = false;
	cweeUnorderedList< cweeRouterRequest >* routerRequests;
	cweeSharedPtr<cweeSysMutex> mut;	
};

void	DoRouterRequest(cweeRouterRequest* io);

class fileLock {
public:
	fileLock() : Name(), lock(new cweeSysMutex()) {};
	fileLock(fileLock const& other) : Name(other.Name), lock(other.lock) {};
	fileLock(fileLock&& other) : Name(other.Name), lock(other.lock) {};
	fileLock& operator=(fileLock const& o) { Name = o.Name; lock = o.lock; return *this; };
	fileLock& operator=(fileLock&& o) { Name = o.Name; lock = o.lock; return *this; };

	cweeStr Name;
	cweeSharedPtr<cweeSysMutex> lock;

	void Lock() {
		lock->Lock();
	};
	void Unlock() {
		lock->Unlock();
	};
};

class cweeFileSystemLocal : public cweeFileSystem {
public:
	virtual	void		Init(void);
	virtual void		resetReadCount(void)	{ readCount.Swap(0); };
	virtual void		addToReadCount(int c)	{ readCount.Swap(((int)readCount) + c); };
	virtual int			getReadCount(int c)		{ return readCount; };

	virtual cweeStr		getAppFolder(void)		{ return appFolder; };
	virtual cweeStr		getAppName(void)		{ return appName; };
	virtual cweeStr		getDataFolder(void)		{ return dataFolder; };
	virtual void		setDataFolder(cweeStr in) { 
		struct stat info; 

		bool success = false;

		if (stat(in, &info) != 0) { 
			success = _mkdir(in);
		}
		else if (!(info.st_mode & S_IFDIR)) { 
			success = _mkdir(in);
		}

		dataFolder = in; 
	};

	virtual void		removeFile(const cweeStr& filePath) {	
		int index = fileLocks.FindIndexWithName(filePath);
		if (index < 0) {
			fileLocks.Lock();
			index = fileLocks.UnsafeAppend();
			auto fPtr = fileLocks.UnsafeRead(index);
			if (fPtr) {
				fPtr->Name = filePath;
			}
			fileLocks.Unlock();
		}
		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Lock();
			}
			fileLocks.AllowDeletion(index);
		}

		{ // do something with the file
			std::remove(filePath);
		}

		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Unlock();
			}
			fileLocks.AllowDeletion(index);
		}
	};
	virtual void		renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath) {
		int index = fileLocks.FindIndexWithName(oldFilePath);
		if (index < 0) {
			fileLocks.Lock();
			index = fileLocks.UnsafeAppend();
			auto fPtr = fileLocks.UnsafeRead(index);
			if (fPtr) {
				fPtr->Name = oldFilePath;
			}
			fileLocks.Unlock();
		}
		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Lock();
			}
			fileLocks.AllowDeletion(index);
		}

		{ // do something with the file
			std::rename(oldFilePath, newFilePath);
		}

		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Unlock();
			}
			fileLocks.AllowDeletion(index);
		}

	};
	virtual bool		ensureDirectoryExists(const cweeStr& Directory) {
		cweeStr directory = Directory;
		if (directory.Find(".") >= 0) {
			directory.ExtractFilePath(directory);
		}

		// ensure the target directory exists.
		struct stat info;

		bool success = false;

		if (stat(directory, &info) != 0)
		{
			success = _mkdir(directory);
		}
		if (!success)
		{
			if (!(info.st_mode & S_IFDIR)) {
				success = _mkdir(directory);
			}
			else {
				success = true;
			}
		}
		return success;
	};
	virtual cweeStr		createFilePath(cweeStr directory, cweeStr fileName, fileType_t fileType);
	virtual cweeThreadedList<cweeStr> readFileAsStrList(cweeStr filePath);
	virtual void		readFileAsCweeStr(cweeStr& out, cweeStr filePath);
	virtual void		writeFileFromStrList(cweeStr filePath, const cweeThreadedList<cweeStr>& content);
	virtual void		writeFileFromCweeStr(cweeStr filePath, const cweeStr& content);
	virtual void		writeFileFromCweeStr(cweeStr folderPath, cweeStr fileName, fileType_t fileType, const cweeStr& content);

	virtual void		setCurrentTime(const u64& time, bool* mode = nullptr) {
		currentTime.Lock();
		auto ptr = currentTime.UnsafeRead();
		if (mode) {
			ptr->first = *mode;
		}
		ptr->second = time;
		currentTime.Unlock();
	};
	virtual u64			getCurrentTime() {
		u64 out;
		
		currentTime.Lock();
		auto ptr = currentTime.UnsafeRead();
		if (ptr->first) {
			out = ptr->second;
		}
		else {		
			out = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
		}
		currentTime.Unlock();
		
		return out;
	};
	virtual cweeTime	localtime(const u64& time) {
		return cweeTime(time);


		//boost::posix_time::ptime _epoch = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		//if (time > 0) {			
		//	_epoch += boost::posix_time::seconds((long long)time);
		//}
		//else {
		//	_epoch -= boost::posix_time::seconds((long long)(-time));
		//}
		//return _epoch;









//#ifdef useFasterMkTime
//		// cweeTime out1({ 0,0,0,0,0,0,0,0,0 });
//		cweeTime out1;
//
//		int index = -1; int numV = 0;
//		timeTolocalTmContainer.Lock();
//		auto ptr = timeTolocalTmContainer.UnsafeRead();
//		if (ptr) {
//			numV = ptr->GetNumValues();
//			index = ptr->FindExactX(time);
//			if (index > 0 && index < numV) {
//				//out1 = ptr->ValueForIndex(index).time;
//				out1 = ptr->ValueForIndex(index);
//				// out1 = x.GetTM();				
//			}
//		}
//		timeTolocalTmContainer.Unlock();
//
//		if (index <= 0 || index >= numV) {
//			time_t t = time;
//			if (t < 0) t = 0;
//			out1 = *std::localtime(&t);
//			{
//				timeTolocalTmContainer.Lock();
//				ptr = timeTolocalTmContainer.UnsafeRead();
//				if (ptr) {
//					ptr->AddUniqueValue(time, out1);
//				}
//				timeTolocalTmContainer.Unlock();
//			}
//		}
//
//		return out1.GetTM();
//#else
//		tm* out; cweeTime out1({ 0,0,0,0,0,0,0,0,0 }); time_t t = time;
//		if (t < 0) t = 0;
//		out = std::localtime(&t);
//
//		// std::chrono::system_clock::from_time_t()
//
//		if (out) {
//			out1 = *out;
//			return out1;
//		}
//		else {
//			return out1;
//		}
//#endif
	};
	virtual cweeTime	gmtime(const u64& time) {
		return cweeTime(time);

		//boost::posix_time::ptime _epoch = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		//if (time > 0) {
		//	_epoch += boost::posix_time::seconds((long long)time);
		//}
		//else {
		//	_epoch -= boost::posix_time::seconds((long long)(-time));
		//}
		//return _epoch;


		//return boost::posix_time::to_tm(_epoch);


//#ifdef useFasterMkTime
//		// cweeTime out1({ 0,0,0,0,0,0,0,0,0 });
//		cweeTime out1;
//
//		int index = -1; int numV = 0;
//		timeTogmTmContainer.Lock();
//		auto ptr = timeTogmTmContainer.UnsafeRead();
//		if (ptr) {
//			numV = ptr->GetNumValues();
//			index = ptr->FindExactX(time);
//			if (index > 0 && index < numV) {
//				//out1 = ptr->ValueForIndex(index).time;
//				out1 = ptr->ValueForIndex(index);
//			}
//		}
//		timeTogmTmContainer.Unlock();
//
//		if (index <= 0 || index >= numV) {
//			tm* out; time_t t = time;
//			if (t < 0) t = 0;
//			out = std::gmtime(&t);
//			if (out) {
//				out1 = *out;
//
//				timeTogmTmContainer.Lock();
//				ptr = timeTogmTmContainer.UnsafeRead();
//				if (ptr) {
//					ptr->AddUniqueValue(time, out1);
//				}
//				timeTogmTmContainer.Unlock();
//			}
//		}
//
//		return out1.GetTM();
//#else
//		tm* out; cweeTime out1({ 0,0,0,0,0,0,0,0,0 }); time_t t = time;
//		if (t < 0) t = 0;
//		out = std::gmtime(&t);
//		if (out) {
//			out1 = *out;
//			return out1;
//		}
//		else {
//			return out1;
//		}
//#endif
	};
	virtual cweeStr		createTimeFromMinutes(float minutes);
	virtual time_t		getFirstDayOfSameMonth(time_t in);
	virtual time_t		getLastDayOfSameMonth(time_t in);
	virtual int			getNumDaysInSameMonth(time_t in);

	virtual cweeStr		DownloadCweeStrFromURL(cweeStr url);
	virtual HRESULT		downloadFileFromURL(cweeStr url, cweeStr destinationFilePath);
	virtual cweeStr		ReadFirstFileFromZipToString(cweeStr zipFilePath);
	virtual cweeStr		ReadFileFromZipToString(cweeStr zipFilePath, int whichStartingZero);
	virtual cweeStr		ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName);
	virtual void		ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName, cweeStr& out);
	virtual void		addContentToZip(cweeStr zipFilePath, const cweeStr& fileName, cweeStr& content);
	virtual void		copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeStr exceptFileName = "");
	virtual void		copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeThreadedList<cweeStr> exceptFileNames);
	virtual void		writeStrListToZipFile(cweeStr zipFilePath, const cweeThreadedList<cweeStr>& content);

	virtual cweeThreadedList<cweeStr> getFileNamesInZip(cweeStr filePath);
	virtual int			getNumFilesInZip(cweeStr zipFilePath);
	virtual void		addFileToZip(cweeStr zipFilePath, cweeStr fileToAddPath);
	virtual void		addFilesToZip(cweeStr zipFilePath, cweeStr fileToAddPath, cweeStr fileToAddPath2 = "", cweeStr fileToAddPath3 = "", cweeStr fileToAddPath4 = "", cweeStr fileToAddPath5="");

	virtual u64			returnTime_gm(int year, int month, int day, int hour, int minute, int second);
	virtual u64			returnTime(int year, int month, int day, int hour, int minute, int second);
	virtual int			getByteSizeOfFile(cweeStr filePath);
	virtual int			listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list);
	virtual std::vector<std::string> listFilesWithExtension(const char* directory, const char* extension);

	virtual void		saveWindowsPassword(cweeStr account, cweeStr username, cweeStr password);
	virtual cweeStr		retrieveWindowsPassword(cweeStr account, cweeStr username);

	virtual cweeStr		appendLog(cweeStr filePath, cweeStr newLine);
	virtual void		copyFile(cweeStr filePathOrig, cweeStr filePathNew);
	virtual bool		checkFileExists(cweeStr filePath);
	virtual int			submitToast(cweeStr title, cweeStr content);
	virtual cweeThreadedList< std::pair<int, std::pair<cweeStr, cweeStr>>> getToasts();
	virtual void		removeToast(int id);

	virtual cweeStr		getExtension(fileType_t fileType);

#pragma region Comma Seperated Values
	virtual std::map<std::string, cweeThreadedList<cweeStr>> readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders = true) {
		std::map<std::string, cweeThreadedList<cweeStr>> out;

		cweeThreadedList<std::string> columnToHeader;
		cweeThreadedList<cweeThreadedList<cweeStr>> parsedContent;
		cweeParser parser;

		std::string get;
		int size(0);

		LockFile(filePath);

		{
			std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
			while (file.good()) {
				getline(file, get);
				if (!cweeStr(get.c_str()).IsEmpty()) {
					size++;
				}
			}
			file.close();


			std::ifstream file2(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
			if (myDataHasHeaders && file2.good()) {
				// get names from first row
				getline(file2, get);
				if (!get.empty()) {
					parser.Parse(get.c_str(), ",", true);
					for (auto& x : parser) {
						columnToHeader.Append(x.c_str());
						parsedContent.Append(cweeThreadedList<cweeStr>(size + 16));
					}
				}
			}

			while (file2.good()) {
				getline(file2, get);
				if (!get.empty()) {
					parser.Parse(get.c_str(), ",", true);
					if (columnToHeader.Num() == 0 && (parser.getNumVars() > columnToHeader.Num())) {
						for (int i = columnToHeader.Num(); i < parser.getNumVars(); i++) {
							columnToHeader.Append(cweeStr::ToString(i).c_str());
							parsedContent.Append(cweeThreadedList<cweeStr>(size + 16));
						}
					}
					for (int i = cweeMath::min(columnToHeader.Num(), parser.getNumVars()) - 1; i >= 0; i--) {
						parsedContent[i].Append(parser[i]);
					}
				}
			}

			file2.close();


		}

		UnlockFile(filePath);

		for (int i = cweeMath::min(columnToHeader.Num(), parsedContent.Num()) - 1; i >= 0; i--) {
			out[columnToHeader[i]].Swap(parsedContent[i]);
		}

		return out;
	};
	virtual std::map<int, cweeThreadedList<std::pair<u64, float>>> readSCADA(const cweeStr& filePath, bool myDataHasHeaders = true) {			
		cweeStr file;
		std::map<int, cweeThreadedList<std::pair<u64, float>>> result;
		this->readFileAsCweeStr(file, filePath);
		{
			cweeInlineParser fileParser, parser;

			int size(0);
			fileParser.Parse(file, "\n");
			size = fileParser.getNumVars();
			if (size <= 0) {
				return result;
			}

			cweeThreadedList < int > tagid(size + 16); tagid.SetNum(size);
			cweeThreadedList < u64 > datetime(size + 16);  datetime.SetNum(size);
			cweeThreadedList < float > value(size + 16); value.SetNum(size);

			int i = 0; int n = 0; int j;
			if (myDataHasHeaders) { i = 1; }
			cweeStr t = "2019-08-01 00:00:00.0000000000";
			for (; i < size; i++, n++) {
				parser.Parse(fileParser[i], ",");
				if (parser.getNumVars() >= 3) {
					tagid[n] = (int)parser[0];
					for (j = 0; j <= 27; j++) {
						t[j] = parser[1][j];
					}
					datetime[n] = (u64)cweeTime::timeFromString(t);
					value[n] = (float)parser[2];
				}
				if (i > 2) { 
					fileParser[i-1].Clear(); 
				}
			}
			parser.Clear();
			fileParser.Clear();
			file.Clear();

			for (i = 0; i < tagid.Num(); i++) {
				auto& temp = result[tagid[i]].Alloc();
				temp.first = datetime[i];
				temp.second = value[i];
			}
		}
		return result;
	};
#pragma endregion

	/*! Start a server that publishes "messages" to listeners that "subscribe" to the port on this machine. There can only be one publisher per port. */
	virtual bool		StartPublishingServer(int Port = 5556);
	/*! End a publishing server at the specified port on this machine. */
	virtual bool		ClosePublishingServer(int Port = 5556);
	/*! Return a list of all ports used by this app on this machine for publishing. */
	virtual cweeThreadedList<int>	GetPublishingServerPorts();
	/*! Send a message out through the specified port to all current subscribers. */
	virtual bool		UpdatePublishingServer(const cweeStr& message, int Port = 5556);

	/*! Start a subscriber on this machine that listens for updates from a publisher at the specified address/port. There can only be one subscriber per machine to any specific address/port combo. */
	virtual bool		StartSubscriptionClient(const cweeStr& address = "localhost", int Port = 5556);
	/*! End a subscriber on this machine listening to the specified address/host. */
	virtual bool		CloseSubscriptionClient(const cweeStr& address = "localhost", int Port = 5556);
	/*! Return a list of all addresses/ports being listened to by this app on this machine. */
	virtual cweeThreadedList<std::pair<cweeStr, int>> GetSubscriptionClients();
	/*!
	Check for any updates from the subscribed publishers. First call after subscribing will always fail. Repeat/frequent calls are highly recommended.
	Allowed to "miss" published messages, as the computer can buffer not-yet-grabbed messages.
	Will only return one message at a time. If the publisher sent out 1000 messages - this must then be called 1000 times to recieve them all.
	*/
	virtual bool		TryGetSubscriptionUpdate(cweeStr& out, const cweeStr& address = "localhost", int Port = 5556);




	virtual bool		StartRouterServer(int Port = 5556);
	virtual bool		CloseRouterServer(int Port = 5556);
	virtual cweeThreadedList<int>	GetRouterServerPorts();
	virtual bool		UpdateRouterServer(int Port = 5556);
	virtual bool		ReplyToRequestClient(void* RouterRequestData);

	virtual bool		StartRequestClient(const cweeStr& address = "localhost", int Port = 5556);
	virtual bool		CloseRequestClient(const cweeStr& address = "localhost", int Port = 5556);
	virtual cweeThreadedList<std::pair<cweeStr, int>> GetRequestClients();
	virtual bool		TrySendRequest(const cweeStr& request, const cweeStr& address = "localhost", int Port = 5556);
	virtual bool		TryGetReply(cweeStr& out, const cweeStr& address = "localhost", int Port = 5556);
	virtual cweeStr		GetReply(const cweeStr& request, const cweeStr& address = "localhost", int Port = 5556);

	virtual cweeThreadedList<std::pair<cweeStr, cweeStr>> ParseJson(cweeStr source, bool* failed = nullptr) {
		cweeThreadedList<std::pair<cweeStr, cweeStr>> out;
		{
#if 0
			cweeStr tempName = cweeStr::printf("temp_%i", cweeRandomInt(1000, 100000));
			chai.add(chaiscript::var((const cweeStr&)source), tempName.c_str());

			cweeStr command; {
				command += "{\n";
				{
					command += cweeStr::printf(R"(
						var asJson = from_json(%s);
						var toReturn = "";
						for (PR : asJson){
							if (toReturn.size() > 0){
								toReturn += "::externalDelim::";
							}
							toReturn += PR.first.to_string() + "::internalDelim::" + PR.second.to_string();
						}
						return toReturn;)", 
						tempName.c_str()
					);
				}
				command += "\n}";
				command.ReduceSpaces(true);
			}
#else
			source.ReplaceInline("\"", "\\\"");

			cweeStr command; {
				command += "{\n";
				{
					// var input = "{\"ip\":\"128.120.151.113\",\"country_code\":\"US\",\"country_name\":\"United States\",\"region_code\":\"CA\",\"region_name\":\"California\",\"city\":\"Davis\",\"zip_code\":\"95616\",\"time_zone\":\"America/Los_Angeles\",\"latitude\":38.5559,\"longitude\":-121.7391,\"metro_code\":862}"
					{
						command += "var& input = \"";
						command += source;
						command += "\";\n";
					}
					command += R"(
						try{
							var& asJson = from_json(input);
							var toReturn = "";
							for (PR : asJson){
								if (toReturn.size() > 0){
									toReturn += "::externalDelim::";
								}
								try{
									toReturn += PR.first.to_string() + "::internalDelim::" + PR.second.to_string();
								}catch(e){}
							}
							return toReturn;
						}catch(ee){ return cweeStr(); }
					)";
				}
				command += "\n}";
				command.ReduceSpaces(true);
			}
#endif
			chaiscript::Boxed_Value reply;
			if (failed) { *failed = true; }
			try{
				reply = chai.eval(command.c_str());
				if (failed) {
					*failed = false;
				}
			}
			catch (chaiscript::Boxed_Value bv) {
				reply = bv;
			}
			catch (chaiscript::exception::eval_error ex) {
				reply = chaiscript::Boxed_Value(std::string(""));
			}
			catch (std::exception ex) {
				reply = chaiscript::Boxed_Value(std::string(""));
			}
			catch (...) {
				reply = chaiscript::Boxed_Value(std::string(""));
			}

			std::string* reply2 = chaiscript::boxed_cast<std::string*>(reply);
			if (reply2) {
				if (failed) {
					*failed = false;
				}

				cweeParser externalParser(reply2->c_str(), "::externalDelim::", true);
				out.SetGranularity(externalParser.getNumVars() + 1);
				cweeParser internalParser;
				for (auto& pr : externalParser) {
					internalParser.Parse(pr, "::internalDelim::", true);
					if (internalParser.getNumVars() >= 2) {
						out.Append(std::pair<cweeStr, cweeStr>(internalParser[0], internalParser[1]));
					}
				}
			}
			else {
				if (failed) {
					*failed = true;
				}
			}
		}
		return out;
	};

	//cweeStr		USPS_FixAddress(const cweeStr& address) {
	//	cweeStr query = "https://secure.shippingapis.com/";
	//	query += "ShippingAPI.dll?API=Verify";
	//	query += "&XML=";
	//	query += "<AddressValidateRequest USERID=\"755UCDAV6485\">";
	//	query += "<Revision>1</Revision>";
	//	query += "<Address ID=\"0\">";
	//	query += "<Address1>SUITE K</Address1>";
	//	query += "<Address2>29851 Aventura</Address2>";
	//	query += "<City/>";
	//	query += "<State>CA</State>";
	//	query += "<Zip5>92688</Zip5>";
	//	query += "<Zip4/>";
	//	query += "</Address>";
	//	query += "</AddressValidateRequest>";


	//	// cweeStr::printf();
	//	query.ReplaceInline(" ", "%20");
	//	"https://secure.shippingapis.com/ShippingAPI.dll?API=Verify&XML=%3CAddressValidateRequest%20USERID=%22755UCDAV6485%22%3E%20%3CRevision%3E1%3C/Revision%3E%20%3CAddress%20ID=%220%22%3E%20%3CAddress1%3ESUITE%20K%3C/Address1%3E%20%3CAddress2%3E29851%20Aventura%3C/Address2%3E%20%3CCity/%3E%20%3CState%3ECA%3C/State%3E%20%3CZip5%3E92688%3C/Zip5%3E%20%3CZip4/%3E%20%3C/Address%3E%20%3C/AddressValidateRequest%3E";
	//
	//};

	std::map<std::string, cweeStr> Geocode_Census(const cweeStr& address) {

		std::map<std::string, cweeStr> out;

		cweeStr parsedAddress = address;
		parsedAddress.ReduceSpaces();
		parsedAddress.Strip(" ");
		parsedAddress.Strip("\n");

		cweeStr content = "\"" + (DownloadCweeStrFromURL(cweeStr::printf("https://geocoding.geo.census.gov/geocoder/locations/onelineaddress?address=%s&benchmark=4&format=json", parsedAddress.ReplaceInline(",", " ").ReplaceInline(" ", "%2C+").c_str())).ReplaceInline("\"", "\\\"")) + "\"";

		try {
			auto bv = chai.eval(cweeStr::printf("return %s.from_json()[\"result\"][\"addressMatches\"];", content.c_str()).c_str());
			// {"result":{"input":{"benchmark":{"id":"2020", "benchmarkName" : "Public_AR_Census2020", "benchmarkDescription" : "Public Address Ranges - Census 2020 Benchmark", "isDefault" : false}, "address" : {"address":"4600 Silver Hill Rd, Washington, DC 20233"}}, "addressMatches" : [{"matchedAddress":"4600 SILVER HILL RD, WASHINGTON, DC, 20233", "coordinates" : {"x":-76.92744, "y" : 38.845985}, "tigerLine" : {"tigerLineId":"76355984", "side" : "L"}, "addressComponents" : {"fromAddress":"4600", "toAddress" : "4700", "preQualifier" : "", "preDirection" : "", "preType" : "", "streetName" : "SILVER HILL", "suffixType" : "RD", "suffixDirection" : "", "suffixQualifier" : "", "city" : "WASHINGTON", "state" : "DC", "zip" : "20233"}}] }}

			chaiscript::small_vector<chaiscript::Boxed_Value>* data = chaiscript::boxed_cast<chaiscript::small_vector<chaiscript::Boxed_Value>*>(bv);
			if (data) {
				for (auto& map_bv : *data) {
					std::map<std::string, chaiscript::Boxed_Value>* data2 = chaiscript::boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(map_bv);
					if (data2) {
						for (auto& category : *data2) {
							switch (cweeStr::hash(category.first.c_str())) {
							case cweeStr::hash("addressComponents"): {
								std::map<std::string, chaiscript::Boxed_Value>* data3 = chaiscript::boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(category.second);
								if (data3) {
									{
										auto sBV = data3->operator[]("fromAddress");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["House Number"].AddToDelimiter(data4->c_str(), "-");
										}
									}
									{
										auto sBV = data3->operator[]("toAddress");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["House Number"].AddToDelimiter(data4->c_str(), "-");
										}
									}
									{
										auto sBV = data3->operator[]("preQualifier");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("preDirection");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("preType");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("streetName");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("suffixType");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("suffixDirection");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("suffixQualifier");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Road"].AddToDelimiter(data4->c_str(), " ");
										}
									}
									{
										auto sBV = data3->operator[]("city");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["City"] = data4->c_str();
										}
									}
									{
										auto sBV = data3->operator[]("state");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["State"] = data4->c_str();
										}
									}
									{
										auto sBV = data3->operator[]("zip");
										std::string* data4 = chaiscript::boxed_cast<std::string*>(sBV);
										if (data4) {
											out["Post Code"] = data4->c_str();
										}
									}
								}
								break;
							}
							case cweeStr::hash("coordinates"): {
								std::map<std::string, chaiscript::Boxed_Value>* data3 = chaiscript::boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(category.second);
								if (data3) {
									{
										auto sBV = data3->operator[]("x");
										double* data4 = chaiscript::boxed_cast<double*>(sBV);
										if (data4) {
											out["Longitude"] = cweeStr(*data4);
										}
									}
									{
										auto sBV = data3->operator[]("y");
										double* data4 = chaiscript::boxed_cast<double*>(sBV);
										if (data4) {
											out["Latitude"] = cweeStr(*data4);
										}
									}

								}
								break;
							}
							case cweeStr::hash("matchedAddress"): {
								std::string* data3 = chaiscript::boxed_cast<std::string*>(category.second);
								if (data3) {
									out["Display Name"] = data3->c_str();
								}
								break;
							}
							}
						}
						break; // first only?
					}
				}
			}
		}
		catch (...) {}

		return out;

	};

	virtual IpAddressInformation GetIpAddressInformation() {
		IpAddressInformation out;
		cweeThreadedList<std::pair<cweeStr, cweeStr>> data;// = ParseJson(geocoding->GetCurrentInternetData());
		cweeStr read;
		bool failed = true;

#pragma region "Internet Data"
		int maxTries = 20;
		while (failed && (--maxTries > 0)) {			
			cweeStr IPaddress;
			{
				cweeStr temp = this->QueryHttp("api.ipify.org", "", "EdmsAppFreeGeoIP"); // ?format=json
				{
					IPaddress = temp;

					//auto split = temp.Split(":");
					//if (split.getNumVars() >= 2) {
					//	IPaddress = split[1];
					//	IPaddress.Strip("\"");
					//}
				}
			}

			cweeStr destinationFilePath = createFilePath(getDataFolder(), cweeStr::printf("temp_%i", cweeRandomInt(1000, 1000000)), fileType_t::TXT);
			{
				CURL* curl;
				FILE* fp;
				CURLcode result;
				curl = curl_easy_init();
				if (curl) {
					{
						int index = fileLocks.FindIndexWithName(destinationFilePath);
						if (index < 0) {
							fileLocks.Lock();
							index = fileLocks.UnsafeAppend();
							auto fPtr = fileLocks.UnsafeRead(index);
							if (fPtr) {
								fPtr->Name = destinationFilePath;
							}
							fileLocks.Unlock();
						}
						if (index >= 0) {
							auto fPtr = fileLocks.GetPtr(index);
							if (fPtr) {
								fPtr->Lock();
							}
						}
					}
					{ // do something with the file
						fp = fopen(destinationFilePath, "wb");
						curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
						curl_easy_setopt(curl, CURLOPT_URL, cweeStr::printf("https://jkosgei-free-ip-geolocation-v1.p.rapidapi.com/%s/english?api-key=1c608fcc8f4fda56661f7d9039121fa0cacce84a74318a18037c65ea", IPaddress.c_str()).c_str());
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileSys_write_data);
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

						struct curl_slist* headers = NULL;
						headers = curl_slist_append(headers, "X-RapidAPI-Host: jkosgei-free-ip-geolocation-v1.p.rapidapi.com");
						headers = curl_slist_append(headers, "X-RapidAPI-Key: 5ad46c78bbmsh509c23a7b43a6d5p1e1963jsn4998d1523143");
						curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

						curl_easy_perform(curl);
						/* always cleanup */
						curl_easy_cleanup(curl);
						fclose(fp);
					}
					{
						int index = fileLocks.FindIndexWithName(destinationFilePath);
						if (index >= 0) {
							auto fPtr = fileLocks.GetPtr(index);
							if (fPtr) {
								fPtr->Unlock();
							}
						}
					}
				}
			}

			readFileAsCweeStr(read, destinationFilePath);

			removeFile(destinationFilePath);

			read.ReplaceInline("\\", "/");

			data = ParseJson(read, &failed);
		}

#pragma endregion
		
		for (auto& x : data) {
			auto& v = x.second;

			if (x.first.Icmp("city") == 0) {
				out.city = v;
			}
			else if (x.first.Icmp("country_code") == 0) {
				out.country_code = v;
			}
			else if (x.first.Icmp("country_name") == 0) {
				out.country_name = v;
			}
			else if (x.first.Icmp("ip") == 0) {
				out.ip = v;
			}
			else if (x.first.Icmp("latitude") == 0) {
				out.latitude = v;
			}
			else if (x.first.Icmp("longitude") == 0) {
				out.longitude = v;
			}
			else if (x.first.Icmp("metro_code") == 0) {
				out.metro_code = v;
			}
			else if (x.first.Icmp("region_code") == 0) {
				out.region_code = v;
			}
			else if (x.first.Icmp("region") == 0) {
				out.region_code = v;
			}
			else if (x.first.Icmp("postal") == 0) {
				out.zip_code = v;
			}
			else if (x.first.Icmp("region_name") == 0) {
				out.region_name = v;
			}
			else if (x.first.Icmp("time_zone") == 0) {
				out.time_zone = v;
			}
			else if (x.first.Icmp("zip_code") == 0) {
				out.zip_code = v;
			}
		}

		out.coordinates = vec3d((double)out.longitude, (double)out.latitude, geocoding->GetElevation(vec2d((double)out.longitude, (double)out.latitude)));

		return out;
	};

	/*! retrieve a time-series of weather data in F */
	virtual cweeThreadedList<std::pair<u64, float>> GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) {
		cweeThreadedList<std::pair<u64, float>> out;
		tm a;
		cweeStr source;
		{		
			cweeStr requestParams;
			{
				auto startTM = this->localtime(start);
				auto endTM = this->localtime(end);

				cweeStr startDate = cweeStr::printf("%i-%i-%i", startTM.tm_year() + 1900, startTM.tm_mon() + 1, startTM.tm_mday()); //"2019-01-01";
				cweeStr endDate = cweeStr::printf("%i-%i-%i", endTM.tm_year() + 1900, endTM.tm_mon() + 1, endTM.tm_mday()); //"2019-01-01";
				cweeStr lat = cweeStr(latitude);
				cweeStr lon = cweeStr(longitude);
				constexpr auto Key = "d09ca182dd8541d7bdd820a45c98d6fc";
				cweeStr key = cweeStr(Key);

				requestParams = cweeStr::printf("weather/?param=temperature&start=%s&end=%s&lat=%s&lon=%s&api-key=%s&freq=D", startDate.c_str(), endDate.c_str(), lat.c_str(), lon.c_str(), key.c_str());
			}

			source = QueryHttp("api.oikolab.com", requestParams, "EdmsAppWeatherQuery");
		}
			
		{
			source.ReplaceInline("\\\"", "||");
			source.ReplaceInline("\"", "\\\"");
			source.ReplaceInline("||", "\\\\\\\"");

#if false // doesn't work. Difficult (though technically possible) to get shared by-reference values back out of chaiscript. 
			auto BV = chaiscript::json_wrap::from_json((cweeStr("\"") + source + cweeStr("\"")).c_str());
			chaiscript::shared_ptr<std::map<std::string, chaiscript::Boxed_Value>> map = chaiscript::boxed_cast<chaiscript::shared_ptr<std::map<std::string, chaiscript::Boxed_Value>>&>(BV);
			chaiscript::shared_ptr<std::string> dataString = chaiscript::boxed_cast<chaiscript::shared_ptr<std::string>&>(map->operator[]("data"));
			BV = chaiscript::json_wrap::from_json(*dataString);
			chaiscript::shared_ptr<std::map<std::string, chaiscript::Boxed_Value>> map2 = chaiscript::boxed_cast<chaiscript::shared_ptr<std::map<std::string, chaiscript::Boxed_Value>>&>(BV);

			chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>> vec_data = chaiscript::boxed_cast<chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>>&>(map2->operator[]("data"));
			chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>> vec_indexes = chaiscript::boxed_cast<chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>>&>(map2->operator[]("index"));
			std::pair<u64, float> v; 
			for (int i = 0; i < vec_data->size(); i++) {
				auto& bv_d = vec_data->operator[](i);
				auto& bv_i = vec_indexes->operator[](i);

				chaiscript::shared_ptr<__int64> X = chaiscript::boxed_cast<chaiscript::shared_ptr<__int64>&>(bv_i);
				chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>> Y_vec = chaiscript::boxed_cast<chaiscript::shared_ptr<chaiscript::small_vector<chaiscript::Boxed_Value>>&>(bv_d);
				if (Y_vec->size() >= 5) {
					chaiscript::shared_ptr<double> Y = chaiscript::boxed_cast<chaiscript::shared_ptr<double>&>(Y_vec->operator[](4));

					v.first = *X;
					v.second = *Y;
					out.Append(v);
				}
			}

#else

			cweeStr command; {
				command += "{\n";
				{
					{
						command += "var input = \"";
						command += source;
						command += "\";\n";
					}
					command += R"(
					var asJson = from_json(input);
					var dataCategory = from_json(asJson["data"]);
					var data := dataCategory["data"];
					var indexes := dataCategory["index"];
					var out = Map(); 
					for (var j = 0; j < data.size(); ++j){
						out[indexes[j].to_string()] := data[j][4];
					}
					for (iter : out){
						iter.second *= 9.0/5.0;
						iter.second += 32.0;
					}
					return out;)";
				}
				command += "\n}";
				command.ReduceSpaces(true);
			}

			try {
				auto BV = chai.eval(command.c_str()); //  scripting->GetBoxedValue

				std::map<std::string, chaiscript::Boxed_Value>* map = chaiscript::boxed_cast<std::map<std::string, chaiscript::Boxed_Value>*>(BV);
				std::pair<u64, float> v;
				if (map) {
					for (auto& x : *map) {
						v.first = (u64)cweeStr(x.first.c_str());
						v.second = chaiscript::boxed_cast<double>(x.second);
						out.Append(v);
					}
				}
			}
			catch (...) {}

#endif
		}
	
		return out;
	};

	virtual cweeStr QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") {
		cweeStr out;
		
		out = geocoding->QueryHttpToFile(mainAddress, requestParameters, UniqueSessionName);

		return out;
	};




	virtual void		Pack_Message(cweeStr& toPackage) {
		toPackage.Append(GetServerMessageSuffix());
	};
	virtual void		Unpack_Message(cweeStr& toUnpackage) {
		if (toUnpackage.Length() >= 6) {
			cweeParser a(toUnpackage, "</WW0>", true);
			if (a.getNumVars() >= 2) {
				toUnpackage = a[0];
			}
		}
		else {
			// could not possibly have a "correct" message, therefore we just report it as-is
		}


	};

private:
	virtual cweeStr		getApplicationPath(void);
	virtual cweeStr		getApplicationName(void);
	virtual void		setAppFolder(cweeStr in) { 
		appFolder = in; };
	virtual void		setAppName(cweeStr in) { 
		appName = in; };
	virtual cweeStr     GetServerMessageSuffix() {
		return "</WW0>";
	};
	virtual zmq::message_t cweeStr_to_zMQ(const cweeStr& message) {
		cweeStr msg = message.c_str();
		Pack_Message(msg);
		zmq::message_t Message(msg.c_str(), msg.Size());
		return Message;
	};
	virtual cweeStr		zMQ_to_cweeStr(const zmq::message_t& message) {
		cweeStr out = static_cast<const char*>(message.data());
		Unpack_Message(out);
		return out;
	};

	virtual cweeStr		getUniqueFingerprint() {
		cweeStr MAC_ADDRESS;
		cweeStr IP_ADDRESS;
		{
			PIP_ADAPTER_INFO AdapterInfo;
			DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

			AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
			if (AdapterInfo != NULL) {
				// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
				if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
					free(AdapterInfo);
					AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
				}
				if (AdapterInfo != NULL) {
					if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
						// Contains pointer to current adapter info
						PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
						do {
							// technically should look at pAdapterInfo->AddressLength
							//   and not assume it is 6.
							MAC_ADDRESS = "";
							for (int i = 0; i < pAdapterInfo->AddressLength; i++) {
								MAC_ADDRESS.AddToDelimiter(cweeStr::printf("%02X", pAdapterInfo->Address[i]), ":");
							}						
							IP_ADDRESS = pAdapterInfo->IpAddressList.IpAddress.String;

							break;

							pAdapterInfo = pAdapterInfo->Next;
						} while (pAdapterInfo);
					}
					free(AdapterInfo);
				}
			}
		}

		cweeStr randomID = cweeStr::printf("%04X-%04X", cweeRandomInt(0x10000), cweeRandomInt(0x10000)); // ideally we don't want this... 

		return MAC_ADDRESS + "-" + IP_ADDRESS + "-" + randomID;
	};

private:
	cweeUnpooledInterlocked < cweePair<bool, u64> >	currentTime = cweePair<bool, u64>(false, time(NULL));
	cweeUnpooledInterlocked < u64 >			timeZoneAdjust = ((boost::posix_time::microsec_clock::local_time() - boost::posix_time::microsec_clock::universal_time()).total_milliseconds() / 1000.0);
	cweeUnpooledInterlocked < boost::posix_time::ptime > epoch = boost::posix_time::time_from_string("1970/1/1 0:0:0");
	cweeUnpooledInterlocked < int >			readCount = 0;
	cweeUnpooledInterlocked < cweeStr >		appFolder = cweeStr("");
	cweeUnpooledInterlocked < cweeStr >		appName = cweeStr("");
	cweeUnpooledInterlocked < cweeStr >		dataFolder = cweeStr("");
	cweeUnorderedList< fileLock >			fileLocks;

	zmq::context_t zeroMQ_context = zmq::context_t(1);
	cweeUnorderedList< cweeSocket > publishingSockets;
	cweeUnorderedList< cweeSocket > subscribingSockets;

	cweeUnorderedList< cweeRouter > routerSockets;
	cweeUnorderedList< cweeSocket > clientSockets;

	cweeStr privateServerKey = "yevUcBl^KyeCO*WUqF6}n:Ci/RzcmqLiT^#>IZ!f"; // public should be == jow0MY5igjre>]VY*P=5COS0$1Ztk]3<R<1MgrPT

	cweeUnorderedList< cweeRouterRequest > routerRequests;

	virtual cweeStr receiveAllMessageParts(zmq::socket_t& socket) {
		cweeStr out;

		while (1) {
			//  Process all parts of the message
			zmq::message_t message;
			auto result = socket.recv(message, zmq::recv_flags::dontwait);
			if (result.has_value()) {
				out.AddToDelimiter(zMQ_to_cweeStr(message), "\n");
			}
			else {
				break;
			}

			int more = 0;           //  Multipart detection
			size_t more_size = sizeof(more);
			socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
			if (!more) break;              //  Last message part
		}

		return out;
	};

#ifdef useFasterMkTime
	class cweeTime
	{
	public:
		cweeTime() : tm_sec(0), tm_min(0), tm_hour(0), tm_mday(0), tm_mon(0), tm_year(0), tm_wday(0), tm_yday(0), tm_isdst(0) {}
		cweeTime(const tm* tm) : tm_sec(tm->tm_sec), tm_min(tm->tm_min), tm_hour(tm->tm_hour), tm_mday(tm->tm_mday), tm_mon(tm->tm_mon), tm_year(tm->tm_year), tm_wday(tm->tm_wday), tm_yday(tm->tm_yday), tm_isdst(tm->tm_isdst) {}
		cweeTime(const tm& tm) : tm_sec(tm.tm_sec), tm_min(tm.tm_min), tm_hour(tm.tm_hour), tm_mday(tm.tm_mday), tm_mon(tm.tm_mon), tm_year(tm.tm_year), tm_wday(tm.tm_wday), tm_yday(tm.tm_yday), tm_isdst(tm.tm_isdst) {}
		//cweeTime& operator=(const cweeTime* tm) {
		//	tm_sec = (tm->tm_sec);
		//	tm_min = (tm->tm_min);
		//	tm_hour = (tm->tm_hour);
		//	tm_mday = (tm->tm_mday);
		//	tm_mon = (tm->tm_mon);
		//	tm_year = (tm->tm_year);
		//	tm_wday = (tm->tm_wday);
		//	tm_yday = (tm->tm_yday);
		//	tm_isdst = (tm->tm_isdst);
		//};
		//cweeTime& operator=(const cweeTime& tm) {
		//	tm_sec = (tm.tm_sec);
		//	tm_min = (tm.tm_min);
		//	tm_hour = (tm.tm_hour);
		//	tm_mday = (tm.tm_mday);
		//	tm_mon = (tm.tm_mon);
		//	tm_year = (tm.tm_year);
		//	tm_wday = (tm.tm_wday);
		//	tm_yday = (tm.tm_yday);
		//	tm_isdst = (tm.tm_isdst);
		//};
		cweeTime& operator=(const tm* tm) {
			tm_sec = (tm->tm_sec);
			tm_min = (tm->tm_min);
			tm_hour = (tm->tm_hour);
			tm_mday = (tm->tm_mday);
			tm_mon = (tm->tm_mon);
			tm_year = (tm->tm_year);
			tm_wday = (tm->tm_wday);
			tm_yday = (tm->tm_yday);
			tm_isdst = (tm->tm_isdst);
			return *this;
		};
		cweeTime& operator=(const tm& tm) {
			tm_sec = (tm.tm_sec);
			tm_min = (tm.tm_min);
			tm_hour = (tm.tm_hour);
			tm_mday = (tm.tm_mday);
			tm_mon = (tm.tm_mon);
			tm_year = (tm.tm_year);
			tm_wday = (tm.tm_wday);
			tm_yday = (tm.tm_yday);
			tm_isdst = (tm.tm_isdst);
			return *this;
		};
		friend bool			operator==(const cweeTime& a, const cweeTime& b) {
			return a.cmp(b);
		};
		friend bool			operator!=(const cweeTime& a, const cweeTime& b) {
			return !a.cmp(b);
		};

		tm GetTM() {
			return cweeTime({ tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst });
		};
		int tm_sec;   // seconds after the minute - [0, 60] including leap second
		int tm_min;   // minutes after the hour - [0, 59]
		int tm_hour;  // hours since midnight - [0, 23]
		int tm_mday;  // day of the month - [1, 31]
		int tm_mon;   // months since January - [0, 11]
		int tm_year;  // years since 1900
		int tm_wday;  // days since Sunday - [0, 6]
		int tm_yday;  // days since January 1 - [0, 365]
		int tm_isdst; // daylight savings time flag
	private :
		bool cmp(const cweeTime& tm) const {
			return 
				(tm_sec == tm.tm_sec) &&
				(tm_min == tm.tm_min) &&
				(tm_hour == tm.tm_hour) &&
				(tm_mday == tm.tm_mday) &&
				(tm_mon == tm.tm_mon) &&
				(tm_year == tm.tm_year) &&
				(tm_wday == tm.tm_wday) &&
				(tm_yday == tm.tm_yday) &&
				(tm_isdst == tm.tm_isdst);
		};
	};

	cweeUnpooledInterlocked<cweeTime>		fastmktime_cache;
	cweeUnpooledInterlocked<time_t>			fastmktime_time_cache = 0;
	time_t fastmktime(cweeTime& tm)
	{
		/*
		 * Copyright (C) 2014 Mitchell Perilstein
		 * Licensed under GNU LGPL Version 3. See LICENSING file for details.
		 */

		time_t result;
		time_t hmsarg;

		/* the epoch time portion of the request */
		hmsarg = 3600 * tm.tm_hour
			+ 60 * tm.tm_min
			+ tm.tm_sec;

		fastmktime_cache.Lock();
		auto& cache = *fastmktime_cache.UnsafeRead();
		if (cache.tm_mday == tm.tm_mday
			&& cache.tm_mon == tm.tm_mon
			&& cache.tm_year == tm.tm_year)
		{
			/* cached - just add h,m,s from request to midnight */
			fastmktime_time_cache.Lock();
			auto& time_cache = *fastmktime_time_cache.UnsafeRead();
			{
				result = time_cache + hmsarg;
			}
			fastmktime_time_cache.Unlock();

			/* Obscure, but documented, return value: only this value in arg struct.
			 *
			 * BUG: dst switchover was computed by mktime() for time 00:00:00
			 * of arg day. So this return value WILL be wrong for switchover days
			 * after the switchover occurs.  There is no clean way to detect this
			 * situation in stock glibc.  This bug will be reflected in unit test
			 * until fixed.  See also github issues #1 and #2.
			 */
			tm.tm_isdst = cache.tm_isdst;
		}
		else
		{
			/* not cached - recompute midnight on requested day */
			cache.tm_mday = tm.tm_mday;
			cache.tm_mon = tm.tm_mon;
			cache.tm_year = tm.tm_year;

			fastmktime_time_cache.Lock();
			auto& time_cache = *fastmktime_time_cache.UnsafeRead();
			{
				auto TM = cache.GetTM();			
				time_cache = std::mktime(&TM);
				tm.tm_isdst = cache.tm_isdst;
				result = (-1 == time_cache) ? -1 : time_cache + hmsarg;
			}
			fastmktime_time_cache.Unlock();
		}
		fastmktime_cache.Unlock();

		return result;
	}

	cweeUnpooledInterlocked<cweeTime>		fastgmmktime_cache;
	cweeUnpooledInterlocked<time_t>			fastgmmktime_time_cache = 0;
	time_t fastmkgmtime(cweeTime& tm)
	{
		/*
		 * Copyright (C) 2014 Mitchell Perilstein
		 * Licensed under GNU LGPL Version 3. See LICENSING file for details.
		 */

		time_t result;
		time_t hmsarg;

		/* the epoch time portion of the request */
		hmsarg = 3600 * tm.tm_hour
			+ 60 * tm.tm_min
			+ tm.tm_sec;

		fastgmmktime_cache.Lock();
		auto& cache = *fastgmmktime_cache.UnsafeRead();
		if (cache.tm_mday == tm.tm_mday
			&& cache.tm_mon == tm.tm_mon
			&& cache.tm_year == tm.tm_year)
		{
			/* cached - just add h,m,s from request to midnight */
			fastgmmktime_time_cache.Lock();
			auto& time_cache = *fastgmmktime_time_cache.UnsafeRead();
			{
				result = time_cache + hmsarg;
			}
			fastgmmktime_time_cache.Unlock();

			/* Obscure, but documented, return value: only this value in arg struct.
			 *
			 * BUG: dst switchover was computed by mktime() for time 00:00:00
			 * of arg day. So this return value WILL be wrong for switchover days
			 * after the switchover occurs.  There is no clean way to detect this
			 * situation in stock glibc.  This bug will be reflected in unit test
			 * until fixed.  See also github issues #1 and #2.
			 */
			tm.tm_isdst = cache.tm_isdst;
		}
		else
		{
			/* not cached - recompute midnight on requested day */
			cache.tm_mday = tm.tm_mday;
			cache.tm_mon = tm.tm_mon;
			cache.tm_year = tm.tm_year;

			fastgmmktime_time_cache.Lock();
			auto& time_cache = *fastgmmktime_time_cache.UnsafeRead();
			{
				auto TM = cache.GetTM();
				time_cache = _mkgmtime(&TM);
				tm.tm_isdst = cache.tm_isdst;
				result = (-1 == time_cache) ? -1 : time_cache + hmsarg;
			}
			fastgmmktime_time_cache.Unlock();
		}
		fastgmmktime_cache.Unlock();

		return result;
	}
		
	//cweeUnpooledInterlocked < cweeCurve<cweeTM> > timeTolocalTmContainer; // since these answers aren't expected to change, cache the responses.
	//cweeUnpooledInterlocked < cweeCurve<cweeTM> > timeTogmTmContainer; // since these answers aren't expected to change, cache the responses.
	cweeUnpooledInterlocked < cweeCurve< cweeTime > > timeTolocalTmContainer; // since these answers aren't expected to change, cache the responses.
	cweeUnpooledInterlocked < cweeCurve< cweeTime > > timeTogmTmContainer; // since these answers aren't expected to change, cache the responses.

#endif
};

cweeFileSystemLocal	fileSystemLocal;
cweeFileSystem* fileSystem = &fileSystemLocal;

void	cweeFileSystemLocal::Init(void) {
	setAppFolder(getApplicationPath());
	setAppName(getApplicationName());
	setDataFolder(appFolder + cweeStr("data\\"));
	resetReadCount();
}

bool		cweeFileSystemLocal::StartPublishingServer(int Port) {
	try {
		int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			// already exists;
			return true;
		}
		else {
			publishingSockets.Lock();
			tryFind = publishingSockets.UnsafeAppend();
			auto ptr = publishingSockets.UnsafeRead(tryFind);
			if (ptr) {
				ptr->address = "";
				ptr->port = Port;
				ptr->Name = cweeStr(Port);
				ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::pub);
				ptr->socket.bind("tcp://*:" + cweeStr(Port)); // may fail if there is already another server. 
			}
			publishingSockets.Unlock();

			return true;
		}
	}
	catch (const std::exception& exc) {		
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::ClosePublishingServer(int Port) {	
	try {
		int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			// already exists;
			publishingSockets.Erase(tryFind);
			return true;
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
cweeThreadedList<int>	cweeFileSystemLocal::GetPublishingServerPorts() {
	cweeThreadedList<int> out;
	try {
		for (auto& x : publishingSockets.GetList()) {
			publishingSockets.Lock();			
			auto ptr = publishingSockets.UnsafeRead(x);
			if (ptr) {
				out.Append(ptr->port);
			}
			publishingSockets.Unlock();
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return out;
	}
	return out;
};
bool		cweeFileSystemLocal::UpdatePublishingServer(const cweeStr& message, int Port) {
	try {
		int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			// already exists;
			zmq::message_t Message = cweeStr_to_zMQ(message);

			publishingSockets.Lock();
			auto ptr = publishingSockets.UnsafeRead(tryFind);
			if (ptr) {
				ptr->socket.send(Message, zmq::send_flags::dontwait);
			}
			publishingSockets.Unlock();

			return true;
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::StartSubscriptionClient(const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = subscribingSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			// already exists;
			return true;
		}
		else {
			subscribingSockets.Lock();
			tryFind = subscribingSockets.UnsafeAppend();
			auto ptr = subscribingSockets.UnsafeRead(tryFind);
			if (ptr) {
				ptr->address = address;
				ptr->port = Port;
				ptr->Name = name;
				ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::sub);
				ptr->socket.connect("tcp://" + name);
				ptr->socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
			}
			subscribingSockets.Unlock();

			return true;
		}

	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::CloseSubscriptionClient(const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = subscribingSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			// already exists;
			subscribingSockets.Erase(tryFind);
			return true;
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
cweeThreadedList<std::pair<cweeStr, int>> cweeFileSystemLocal::GetSubscriptionClients() {
	cweeThreadedList<std::pair<cweeStr, int>> out;
	try {
		for (auto& x : subscribingSockets.GetList()) {
			subscribingSockets.Lock();
			auto ptr = subscribingSockets.UnsafeRead(x);
			if (ptr) {
				out.Append(std::pair<cweeStr, int>(ptr->address.c_str(), ptr->port));
			}
			subscribingSockets.Unlock();
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return out;
	}
	return out;
};
bool		cweeFileSystemLocal::TryGetSubscriptionUpdate(cweeStr& out, const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = subscribingSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			// already exists;
			zmq::recv_result_t result;
			zmq::message_t update;
			{
				subscribingSockets.Lock();
				auto ptr = subscribingSockets.UnsafeRead(tryFind);
				if (ptr) {
					result = ptr->socket.recv(update, zmq::recv_flags::dontwait);
				}
				subscribingSockets.Unlock();
			}

			if (result.has_value()) {
				out = zMQ_to_cweeStr(update);
				// data is available
				return true;
			}
			else {
				// no data available
				return false;
			}		
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};

bool		cweeFileSystemLocal::StartRouterServer(int Port) {
	try {
		int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			// already exists;
			return true;
		}
		else {
			routerSockets.Lock();
			tryFind = routerSockets.UnsafeAppend();
			auto ptr = routerSockets.UnsafeRead(tryFind);
			if (ptr) {
				ptr->frontend_port = Port;
				ptr->Name = cweeStr(Port);
				ptr->backend_name = cweeStr(Port) + "backend";

				ptr->frontend_socket =	zmq::socket_t(zeroMQ_context, zmq::socket_type::router);
				ptr->backend_socket =	zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);
				ptr->backend_worker_socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);

#ifdef useCurveSecurity

				char serverPublicKey[41], serverSecretKey[41];
				for (int i = 0 ; i < 41; i++) serverSecretKey[i] = privateServerKey[i];
				
				{			
					if (zmq_curve_public(serverPublicKey, serverSecretKey) == 0) {
						ptr->frontend_socket.setsockopt(ZMQ_CURVE_SERVER, 1);
						ptr->frontend_socket.setsockopt(ZMQ_CURVE_PUBLICKEY, serverPublicKey, strlen(serverPublicKey));
						ptr->frontend_socket.setsockopt(ZMQ_CURVE_SECRETKEY, serverSecretKey, strlen(serverSecretKey));
						ptr->frontend_socket_secretKey = serverSecretKey;

						submitToast("Public Server Key", serverPublicKey);
						submitToast("Private Server Key", serverSecretKey);
					}
				}
\
#endif

				ptr->frontend_socket.bind("tcp://*:" + cweeStr(Port));
				ptr->backend_socket.bind("inproc://:" + ptr->backend_name);				
				ptr->backend_worker_socket.connect("inproc://:" + ptr->backend_name);

				ptr->proxyThread = std::thread(std::bind(&cweeRouter::StartRouterProxy, ptr));
			}
			routerSockets.Unlock();

			return true;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::CloseRouterServer(int Port) {
	try {
		int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			// already exists;
			routerSockets.Erase(tryFind);
			return true;
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
cweeThreadedList<int>	cweeFileSystemLocal::GetRouterServerPorts() {
	cweeThreadedList<int> out;
	try {
		for (auto& x : routerSockets.GetList()) {
			routerSockets.Lock();
			auto ptr = routerSockets.UnsafeRead(x);
			if (ptr) {
				out.Append(ptr->frontend_port);
			}
			routerSockets.Unlock();
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return out;
	}
	return out;
};
bool		cweeFileSystemLocal::UpdateRouterServer(int Port) {
	try {
		int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
		if (tryFind >= 0) {
			zmq::recv_result_t result;
			zmq::message_t id;
			cweeStr identity;
			zmq::message_t msg;
			cweeStr request;
			zmq::message_t copied_id;
			zmq::message_t outbound_msg;
			
			bool success = false;

			{
				routerSockets.Lock();
				auto ptr = routerSockets.UnsafeRead(tryFind);
				if (ptr) {					
					result = ptr->backend_worker_socket.recv(id, zmq::recv_flags::dontwait);
					if (result.has_value()) {
						ptr->backend_worker_socket.recv(&msg);
						success = true;
					}
				}
				routerSockets.Unlock();
			}
			
			if (success) {
				request = zMQ_to_cweeStr(msg);

				routerRequests.Lock();
				int index = routerRequests.UnsafeAppend();
				auto reqPtr = routerRequests.UnsafeRead(index);
				if (reqPtr) {
					reqPtr->Lock(); {
						reqPtr->request = request;
						reqPtr->requester_id.copy(id);
						reqPtr->request_index = index;
						reqPtr->routerPort = cweeStr(Port);
						reqPtr->routerRequests = &routerRequests;
					} reqPtr->Unlock();

					cweeMultithreading::ADD_JOB(DoRouterRequest, reqPtr);
				}
				routerRequests.Unlock();

				return true;
			}
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::ReplyToRequestClient(void* RouterRequestData) {
	bool success = false;
	if (RouterRequestData) {
		cweeRouterRequest& data = *((cweeRouterRequest*)RouterRequestData);
		data.Lock(); {
			int tryFind = routerSockets.FindIndexWithName(data.routerPort);
			if (tryFind >= 0) {
				auto outbound_msg = cweeStr_to_zMQ(data.reply);
				routerSockets.Lock();
				auto ptr = routerSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->backend_worker_socket.send(data.requester_id, ZMQ_SNDMORE);
					ptr->backend_worker_socket.send(outbound_msg);
					success = true;
				}
				routerSockets.Unlock();
			}
		} data.Unlock();
	}
	return success;
};

bool		cweeFileSystemLocal::StartRequestClient(const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = clientSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			// already exists;
			return true;
		}
		else {
			clientSockets.Lock();
			tryFind = clientSockets.UnsafeAppend();
			auto ptr = clientSockets.UnsafeRead(tryFind);
			if (ptr) {
				ptr->address = address;
				ptr->port = Port;
				ptr->Name = name;
				ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);

				cweeStr identity =
					// cweeStr::printf("%04X-%04X", cweeRandomInt(0x10000), cweeRandomInt(0x10000));
					getUniqueFingerprint();

				Pack_Message(identity);

				ptr->socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), strlen(identity));


#ifdef useCurveSecurity

				char serverPublicKey[41], serverSecretKey[41];
				for (int i = 0; i < 41; i++) serverSecretKey[i] = privateServerKey[i];
				zmq_curve_public(serverPublicKey, serverSecretKey);

				char clientPublicKey[41], clientSecretKey[41];
				{
					if (zmq_curve_keypair(clientPublicKey, clientSecretKey) == 0) { // no error on 0 return
						ptr->socket.setsockopt(ZMQ_CURVE_SERVERKEY, serverPublicKey, strlen(serverPublicKey));
						ptr->socket.setsockopt(ZMQ_CURVE_PUBLICKEY, clientPublicKey, strlen(clientPublicKey));
						ptr->socket.setsockopt(ZMQ_CURVE_SECRETKEY, clientSecretKey, strlen(clientSecretKey));
					}
				}

#endif


				ptr->socket.connect("tcp://" + name);
			}
			clientSockets.Unlock();

			return true;
		}

	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::CloseRequestClient(const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = clientSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			// already exists;
			clientSockets.Erase(tryFind);
			return true;
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
cweeThreadedList<std::pair<cweeStr, int>> cweeFileSystemLocal::GetRequestClients() {
	cweeThreadedList<std::pair<cweeStr, int>> out;
	try {
		for (auto& x : clientSockets.GetList()) {
			clientSockets.Lock();
			auto ptr = clientSockets.UnsafeRead(x);
			if (ptr) {
				out.Append(std::pair<cweeStr, int>(ptr->address.c_str(), ptr->port));
			}
			clientSockets.Unlock();
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return out;
	}
	return out;
};
bool		cweeFileSystemLocal::TrySendRequest(const cweeStr& request, const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = clientSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			auto req = cweeStr_to_zMQ(request);
			bool success = false;
			zmq::recv_result_t result;
			{
				clientSockets.Lock();
				auto ptr = clientSockets.UnsafeRead(tryFind);
				if (ptr && !(ptr->awaitingReply)) {
					result = ptr->socket.send(req, zmq::send_flags::dontwait);		
					ptr->awaitingReply = true;
					success = true;
				}
				clientSockets.Unlock();
			}

			return success;

			//if (result.has_value()) {
			//	// request successfully sent
			//	return true;
			//}
			//else {
			//	// request failed to send - (i.e. previous request was already made)
			//	return false;
			//}
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
bool		cweeFileSystemLocal::TryGetReply(cweeStr& out, const cweeStr& address, int Port) {
	try {
		cweeStr name = address + ":" + cweeStr(Port);
		int tryFind = clientSockets.FindIndexWithName(name);
		if (tryFind >= 0) {
			bool success = false;
			{
				clientSockets.Lock();
				auto ptr = clientSockets.UnsafeRead(tryFind);
				if (ptr && ptr->awaitingReply) {
					zmq::pollitem_t items[] = { { ptr->socket, 0, ZMQ_POLLIN, 0 } };

					zmq::poll(items, 1, 10); // 10 milliseconds?

					if (items[0].revents & ZMQ_POLLIN) {
						out = receiveAllMessageParts(ptr->socket);	
						ptr->awaitingReply = false;
						success = true;
					}
				}
				clientSockets.Unlock();
			}

			if (success) {
				// data is available
				return true;
			}
			else {
				// no data available
				return false;
			}
		}
		else {
			// does not exist;
			return false;
		}
	}
	catch (const std::exception& exc) {
		submitToast("ZeroMQ Error", exc.what());
		return false;
	}
	return false;
};
cweeStr		cweeFileSystemLocal::GetReply(const cweeStr& request, const cweeStr& address, int Port) {
	cweeStr out;

	if (TrySendRequest(request, address, Port)) {
		while (true) {
			if (TryGetReply(out, address, Port)) break;
		}
		// got 'out'
	}
	else {
		if (TryGetReply(out, address, Port)) return out;
		else out = "Request Could Not Be Made";
	}

	return out;
};




cweeStr	cweeFileSystemLocal::getExtension(fileType_t fileType) {

	switch (fileType) {
		case INP:{
			return ".inp";
			break;
		}
		case TXT: {
			return ".txt";
			break;
		}
		case CSV: {
			return ".csv";
			break;
		}
		case EXL: {
			return ".xlxs";
			break;
		}
		case DAT: {
			return ".dat";
			break;
		}
		case ZIP: {
			return ".zip";
			break;
		}
		case ZIP_7z: {
			return ".7z";
			break;
		}
		case sqlDB: {
			return ".db";
			break;
		}
		case EDMS: {
			return ".edmsTxt";
			break;
		}
		case PK4: {
			return ".pk4";
			break;
		}
		case PROJECT: {
			return ".water";
			break;
		}
		case PATFILE: {
			return ".pattern";
			break;
		}
	}
	return ".*";
}

cweeStr	cweeFileSystemLocal::createFilePath(cweeStr directory, cweeStr fileName, fileType_t fileType) {
	// ensure the target directory exists.
	struct stat info; 

	bool success = false;

	if (stat(directory, &info) != 0) 
	{ 
		success = _mkdir(directory);
	} 
	else if (!(info.st_mode & S_IFDIR)) 
	{	
		success = _mkdir(directory);
	}
	return directory + "\\" + fileName + getExtension(fileType);
}

cweeStr	cweeFileSystemLocal::createTimeFromMinutes(float minutes) {
	cweeStr out;
	int simHours = cweeMath::Floor(minutes / 60);
	int simMinutes = cweeMath::Floor(cweeMath::Frac(minutes / 60) * 60);
	if (simHours < 10) out = "0" + cweeStr(simHours); else out = cweeStr(simHours);
	out += ":";
	if (simMinutes < 10) out += cweeStr("0" + cweeStr(simMinutes)); else out += cweeStr(simMinutes);
	return out;
}

time_t		cweeFileSystemLocal::getFirstDayOfSameMonth(time_t in) {
	time_t localTime = (time_t)in;
#ifdef useFasterMkTime
	tm tmp = gmtime(localTime);
	return returnTime( // localTime
		tmp.tm_year + 1900,
		tmp.tm_mon + 1,
		1,
		0,
		0,
		0);
#else
	tm* tmp = std::gmtime(&localTime);
	return returnTime( // localTime
		tmp->tm_year + 1900,
		tmp->tm_mon + 1,
		1,
		0,
		0,
		0);



#endif
};
time_t		cweeFileSystemLocal::getLastDayOfSameMonth(time_t in) {
	time_t localTime = (time_t)in;
#ifdef useFasterMkTime
	auto tmp = gmtime(localTime);
	return returnTime( // localTime
		tmp.tm_year + 1900,
		tmp.tm_mon + 2,
		1,
		0,
		0,
		0) - 1;
#else
	auto tmp = gmtime(localTime);
	if ((tmp.tm_mon() + 1) < 12) {
		return returnTime( // localTime
			tmp.tm_year() + 1900,
			tmp.tm_mon() + 2,
			1,
			0,
			0,
			0) - 1;
	}
	else {
		return returnTime( // localTime
			tmp.tm_year() + 1901,
			1,
			1,
			0,
			0,
			0) - 1;
	}
#endif



};

int			cweeFileSystemLocal::getNumDaysInSameMonth(time_t in) {
	u64 t0 = getFirstDayOfSameMonth(in);
	u64 t1 = getLastDayOfSameMonth(in);
	u64 tD = t1 - t0;
	tD /= (24 * 60 * 60);
	return cweeMath::Rint(tD);
};


struct downloadFilePackage {
	cweeStr url;
	cweeStr destinationFilePath;
	cweeUnorderedList< fileLock >*			fileLocks = nullptr;
};
void multithreaded_downloadFile(downloadFilePackage* ptr);
void multithreaded_downloadFile(downloadFilePackage* ptr) {

	CURL* curl;
	FILE* fp;
	CURLcode result;
	char* Url = (char*)ptr->url.c_str();

	curl = curl_easy_init();
	if (curl) {
		if (ptr->fileLocks) {
			int index = ptr->fileLocks->FindIndexWithName(ptr->destinationFilePath);
			if (index < 0) {
				ptr->fileLocks->Lock();
				index = ptr->fileLocks->UnsafeAppend();
				auto fPtr = ptr->fileLocks->UnsafeRead(index);
				if (fPtr) {
					fPtr->Name = ptr->destinationFilePath;
				}
				ptr->fileLocks->Unlock();				
			}
			if (index >= 0) {
				ptr->fileLocks->PreventDeletion(index);
				ptr->fileLocks->Lock();
				auto fPtr = ptr->fileLocks->UnsafeRead(index);
				ptr->fileLocks->Unlock();
				if (fPtr) {
					fPtr->Lock();
				}
				ptr->fileLocks->AllowDeletion(index);
			}
		}
		{ // do something with the file
			fp = fopen(ptr->destinationFilePath, "wb");
			curl_easy_setopt(curl, CURLOPT_URL, Url);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileSys_write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_perform(curl);
			/* always cleanup */
			curl_easy_cleanup(curl);
			fclose(fp);
		}
		if (ptr->fileLocks) {
			int index = ptr->fileLocks->FindIndexWithName(ptr->destinationFilePath);
			if (index >= 0) {
				ptr->fileLocks->PreventDeletion(index);
				ptr->fileLocks->Lock();
				auto fPtr = ptr->fileLocks->UnsafeRead(index);
				ptr->fileLocks->Unlock();
				if (fPtr) {
					fPtr->Unlock();
				}				
				ptr->fileLocks->AllowDeletion(index);
			}
		}
	}

	if (ptr)
		delete ptr;
};
REGISTER_WITH_PARALLEL_THREAD(multithreaded_downloadFile);

HRESULT cweeFileSystemLocal::downloadFileFromURL(cweeStr url, cweeStr destinationFilePath) {
	ensureDirectoryExists(destinationFilePath);
	downloadFilePackage* io = new downloadFilePackage; {
		io->url = url;
		io->destinationFilePath = destinationFilePath;
		io->fileLocks = &fileLocks;
	}
	multithreaded_downloadFile(io);
	return S_OK;

//	LPCSTR URL_CharStr = (LPCSTR)url.c_str();
//	LPCSTR destination_CharStr = (LPCSTR)destinationFilePath.c_str();
//	return URLDownloadToFileA(NULL, URL_CharStr, destination_CharStr, NULL, NULL);
}

cweeStr		cweeFileSystemLocal::DownloadCweeStrFromURL(cweeStr url) {
	cweeStr randomFilePath = fileSystem->createFilePath(fileSystem->getDataFolder(), cweeStr(cweeRandomInt(1000, 1000000)) + "_DWNLD", CSV);

	downloadFileFromURL(url, randomFilePath);

	cweeStr content;
	readFileAsCweeStr(content, randomFilePath);

	removeFile(randomFilePath);

	return content;
}



u64		cweeFileSystemLocal::returnTime_gm(int year, int month, int day, int hour, int minute, int second) {
	return returnTime(year, month, day, hour, minute, second);
}

u64		cweeFileSystemLocal::returnTime(int year, int month, int day, int hour, int minute, int second) {
	
	u64 out = 0;
	try {
		//boost::posix_time::ptime local = boost::posix_time::microsec_clock::local_time();
		//boost::posix_time::ptime univ = boost::posix_time::microsec_clock::universal_time();

		// boost::posix_time::ptime timeObj = boost::posix_time::time_from_string("2016/2/19 9:01:33.10");
		boost::posix_time::ptime timeObj = boost::posix_time::time_from_string(cweeStr::printf("%i/%i/%i 0:0:0", year, month, day).c_str());
		// boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970/1/1 0:0:0");
		epoch.Lock();
		if (timeObj >= *epoch.UnsafeRead()) {
			out = (timeObj - *epoch.UnsafeRead()).total_milliseconds() / 1000.0;
		}
		else {
			out = -1.0 * ((*epoch.UnsafeRead() - timeObj).total_milliseconds() / 1000.0);
		}
		epoch.Unlock();

		out += hour * 3600;
		out += minute * 60;
		out += second;

		timeZoneAdjust.Lock();
		out -= *timeZoneAdjust.UnsafeRead(); // ((local - univ).total_milliseconds() / 1000.0);
		timeZoneAdjust.Unlock();
	}
	catch (...) {
		// bad input.

		time_t rawtime = getCurrentTime();
		struct tm timeinfo({ 0,0,0,0,0,0,0,0,0 });
		timeinfo = *std::gmtime(&rawtime);
		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;
		rawtime = _mkgmtime(&timeinfo);
		if (rawtime < 0) rawtime = getCurrentTime();
		timeinfo = *std::localtime(&rawtime);
		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0; // was 0?
		timeinfo.tm_sec = 0; // was 0?
		rawtime = mktime(&timeinfo);
		rawtime += hour * 3600;
		rawtime += minute * 60;
		rawtime += second;

		return rawtime;
	}
	return out;
	
//	time_t rawtime = getCurrentTime();
//	{		
//		/* get current timeinfo and modify it to the user's choice */
//#ifdef useFasterMkTime
//		cweeTime timeinfo;
//		timeinfo = gmtime(rawtime);
//#else
//		struct tm timeinfo({ 0,0,0,0,0,0,0,0,0 });
//		timeinfo = *std::gmtime(&rawtime);
//#endif
//		timeinfo.tm_year = year - 1900;
//		timeinfo.tm_mon = month - 1;
//		timeinfo.tm_mday = day;
//		timeinfo.tm_hour = 0;
//		timeinfo.tm_min = 0; // was 0?
//		timeinfo.tm_sec = 0; // was 0?
//
//#ifdef useFasterMkTime
//		// rawtime = fastmkgmtime(timeinfo);
//
//		{
//			/*
//			 * Copyright (C) 2014 Mitchell Perilstein
//			 * Licensed under GNU LGPL Version 3. See LICENSING file for details.
//			 */
//
//			time_t result;
//			time_t hmsarg;
//
//			/* the epoch time portion of the request */
//			hmsarg = 3600 * timeinfo.tm_hour
//				+ 60 * timeinfo.tm_min
//				+ timeinfo.tm_sec;
//
//			fastgmmktime_cache.Lock();
//			auto& cache = *fastgmmktime_cache.UnsafeRead();
//			if (cache.tm_mday == timeinfo.tm_mday
//				&& cache.tm_mon == timeinfo.tm_mon
//				&& cache.tm_year == timeinfo.tm_year)
//			{
//				/* cached - just add h,m,s from request to midnight */
//				fastgmmktime_time_cache.Lock();
//				auto& time_cache = *fastgmmktime_time_cache.UnsafeRead();
//				{
//					result = time_cache + hmsarg;
//				}
//				fastgmmktime_time_cache.Unlock();
//
//				/* Obscure, but documented, return value: only this value in arg struct.
//				 *
//				 * BUG: dst switchover was computed by mktime() for time 00:00:00
//				 * of arg day. So this return value WILL be wrong for switchover days
//				 * after the switchover occurs.  There is no clean way to detect this
//				 * situation in stock glibc.  This bug will be reflected in unit test
//				 * until fixed.  See also github issues #1 and #2.
//				 */
//				timeinfo.tm_isdst = cache.tm_isdst;
//			}
//			else
//			{
//				/* not cached - recompute midnight on requested day */
//				cache.tm_mday = timeinfo.tm_mday;
//				cache.tm_mon = timeinfo.tm_mon;
//				cache.tm_year = timeinfo.tm_year;
//
//				fastgmmktime_time_cache.Lock();
//				auto& time_cache = *fastgmmktime_time_cache.UnsafeRead();
//				{
//					auto TM = cache.GetTM();
//					time_cache = _mkgmtime(&TM);
//					timeinfo.tm_isdst = cache.tm_isdst;
//					result = (-1 == time_cache) ? -1 : time_cache + hmsarg;
//				}
//				fastgmmktime_time_cache.Unlock();
//			}
//			fastgmmktime_cache.Unlock();
//
//			rawtime = result;
//		}
//
//#else
//		rawtime = _mkgmtime(&timeinfo);
//#endif
//		
//	}
//	if (rawtime < 0) rawtime = getCurrentTime();
//	{
//#ifdef useFasterMkTime
//		cweeTime timeinfo;
//#else
//		struct tm timeinfo({ 0,0,0,0,0,0,0,0,0 });
//#endif
//
//		/* get current timeinfo and modify it to the user's choice */
//		timeinfo = localtime(rawtime);
//		timeinfo.tm_year = year - 1900;
//		timeinfo.tm_mon = month - 1;
//		timeinfo.tm_mday = day;
//		timeinfo.tm_hour = 0;
//		timeinfo.tm_min = 0; // was 0?
//		timeinfo.tm_sec = 0; // was 0?
//
//#ifdef useFasterMkTime
//		// rawtime = fastmktime(timeinfo);
//
//		{
//			/*
//			 * Copyright (C) 2014 Mitchell Perilstein
//			 * Licensed under GNU LGPL Version 3. See LICENSING file for details.
//			 */
//
//			time_t result;
//			time_t hmsarg;
//
//			/* the epoch time portion of the request */
//			hmsarg = 3600 * timeinfo.tm_hour
//				+ 60 * timeinfo.tm_min
//				+ timeinfo.tm_sec;
//
//			fastmktime_cache.Lock();
//			auto& cache = *fastmktime_cache.UnsafeRead();
//			if (cache.tm_mday == timeinfo.tm_mday
//				&& cache.tm_mon == timeinfo.tm_mon
//				&& cache.tm_year == timeinfo.tm_year)
//			{
//				/* cached - just add h,m,s from request to midnight */
//				fastmktime_time_cache.Lock();
//				auto& time_cache = *fastmktime_time_cache.UnsafeRead();
//				{
//					result = time_cache + hmsarg;
//				}
//				fastmktime_time_cache.Unlock();
//
//				/* Obscure, but documented, return value: only this value in arg struct.
//				 *
//				 * BUG: dst switchover was computed by mktime() for time 00:00:00
//				 * of arg day. So this return value WILL be wrong for switchover days
//				 * after the switchover occurs.  There is no clean way to detect this
//				 * situation in stock glibc.  This bug will be reflected in unit test
//				 * until fixed.  See also github issues #1 and #2.
//				 */
//				timeinfo.tm_isdst = cache.tm_isdst;
//			}
//			else
//			{
//				/* not cached - recompute midnight on requested day */
//				cache.tm_mday = timeinfo.tm_mday;
//				cache.tm_mon = timeinfo.tm_mon;
//				cache.tm_year = timeinfo.tm_year;
//
//				fastmktime_time_cache.Lock();
//				auto& time_cache = *fastmktime_time_cache.UnsafeRead();
//				{
//					auto TM = cache.GetTM();
//					time_cache = std::mktime(&TM);
//					timeinfo.tm_isdst = cache.tm_isdst;
//					result = (-1 == time_cache) ? -1 : time_cache + hmsarg;
//				}
//				fastmktime_time_cache.Unlock();
//			}
//			fastmktime_cache.Unlock();
//
//			rawtime = result;
//		}
//
//#else
//		rawtime = mktime(&timeinfo);
//#endif
//	}
//		
//	return rawtime + (3600.0 * hour) + (60.0 * minute) + (second);
}

int	cweeFileSystemLocal::getByteSizeOfFile(cweeStr filePath) {
	std::streampos begin, end;

	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}
	
	{ // do something with the file
		std::ifstream file(filePath, std::ios::binary);
		begin = file.tellg();
		file.seekg(0, std::ios::end);
		end = file.tellg();
		file.close();
	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}
	
	return (end - begin);
}

cweeStr cweeFileSystemLocal::ReadFirstFileFromZipToString(cweeStr zipFilePath) {

	unzipper zipFile;


	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	
	zipFile.open(zipFilePath);
	cweeStr digitized;
	int numFiles = 0;
	for (std::string filename : zipFile.getFilenames())
	{
		if (numFiles > 0) break;
		cweeStr tempFile = fileSystem->createFilePath(fileSystem->getDataFolder(), cweeStr(cweeRandomInt(1000, 10000)), TXT);

		zipFile.openEntry(filename.c_str());
		std::ofstream wFile;
		wFile.open(tempFile.c_str());
		zipFile >> wFile;
		wFile.close();

		fileSystem->readFileAsCweeStr(digitized, tempFile);
		remove(tempFile.c_str());			

		digitized.Replace("\n\n", "\n"); // why is this even a thing
		numFiles++;
	}
	zipFile.close();

		
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}

	return digitized;
}

bool compareFunction(std::string a, std::string b) { return a < b; }

cweeStr cweeFileSystemLocal::ReadFileFromZipToString(cweeStr zipFilePath, int whichStartingZero) {
#define unzip_inMem
	unzipper zipFile;


	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}





	zipFile.open(zipFilePath);
	cweeStr digitized;
	int numFiles = 0;

	auto list = zipFile.getFilenames();

	std::sort(list.begin(), list.end(), compareFunction); //sort the vector alphebetically

	for (auto& filename : list)
	{
		if (numFiles > whichStartingZero) {
			break;
		}
#ifndef unzip_inMem
		cweeStr tempFile = fileSystem->createFilePath(fileSystem->getDataFolder(), cweeStr(cweeRandomInt(1000, 10000)), TXT);
#endif

		{			
#ifdef unzip_inMem
			if (numFiles == whichStartingZero) {
				zipFile.openEntry(filename.c_str());
				zipFile.ReadEntry(digitized);
				digitized.Replace("\n\n", "\n"); // why is this even a thing
				zipFile.closeEntry();
			}
#else
			zipFile.openEntry(filename.c_str());
			std::ofstream wFile;
			wFile.open(tempFile.c_str());
			zipFile >> wFile;
			wFile.close();
			zipFile.closeEntry();
#endif			
		}

#ifndef unzip_inMem
		if (numFiles == whichStartingZero) {
			fileSystem->readFileAsCweeStr(digitized, tempFile);
		}
		remove(tempFile.c_str());
		digitized.Replace("\n\n", "\n"); // why is this even a thing
#endif
	
		numFiles++;		
	}
	zipFile.close();
	// if (zipFile.isOpen()) throw;


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}

	return digitized;
#undef unzip_inMem
}

cweeStr	cweeFileSystemLocal::ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName) {
	cweeStr out;
	unzipper UnzipFile;

	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	{ // do something with the file
		if (UnzipFile.open(zipFilePath))
		{
			if (UnzipFile.openEntry((fileName + ".txt").c_str())) {
				UnzipFile.ReadEntry(out);
			}
			UnzipFile.closeEntry();
		}
		UnzipFile.close();
	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}






	return out;
};

void	cweeFileSystemLocal::ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName, cweeStr& out) {
	out.Clear();

	unzipper UnzipFile;

	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	{ // do something with the file
		if (UnzipFile.open(zipFilePath))
		{
			if (UnzipFile.openEntry((fileName + ".txt").c_str())) {
				UnzipFile.ReadEntry(out);
			}
			UnzipFile.closeEntry();
		}
		UnzipFile.close();
	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}
};

void	cweeFileSystemLocal::addContentToZip(cweeStr zipFilePath, const cweeStr& fileName, cweeStr& content) {
	ensureDirectoryExists(zipFilePath);
#if 0
	// step 1. Get a copy of all the previous contents. 
	cweeThreadedList<cweeStr> data; 
	cweeThreadedList<cweeStr> labels;
	{
		unzipper UnzipFile;
		if (UnzipFile.open(zipFilePath))
		{
			for (auto& x : UnzipFile.getFilenames()) {
				if (cweeStr(x.c_str()) != (fileName + ".txt")) { // don't copy the file we intend to replace.
					labels.AddUnique(x.c_str());
				}
			}
			for (auto& x : labels) {
				if (x != (fileName + ".txt")) { // don't copy the file we intend to replace.
					if (UnzipFile.openEntry(x.c_str())) {
						data.Append(UnzipFile.ReadEntry().c_str());
					}
					//else {
					//	cweeStr d;
					//	if (d != x) {
					//		ijk++;
					//	}
					//}

					UnzipFile.closeEntry();
				}
			}
		}
		UnzipFile.close();
	}

	// step 2. add the old content back in to the new zip file, AND the new content simultaneously. 
	{
		zipper ZipFile;
		if (ZipFile.open(zipFilePath)) {	
			ZipFile.close(); // empty it? 
			ZipFile.open(zipFilePath);

			if (ZipFile.addEntry((fileName + ".txt").c_str())) 
				ZipFile << content.c_str();	
			content.Clear();
			ZipFile.closeEntry();

			for (int i = 0; i < labels.Num(); i++) {
				if (labels[i] != (fileName + ".txt")) {
					if (ZipFile.addEntry(labels[i])) 
						ZipFile << data[i].c_str();
					data[i].Clear();
					ZipFile.closeEntry();
				}
			}
		}
		ZipFile.close();
	}
#else
	zipper ZipFile;

	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	{ // do something with the file
		if (ZipFile.open(zipFilePath, true)) {
			if (ZipFile.addEntry((fileName + ".txt").c_str())) ZipFile << content.c_str();
			content.Clear();
			ZipFile.closeEntry();
		}
		else {
			if (ZipFile.open(zipFilePath)) {
				if (ZipFile.addEntry((fileName + ".txt").c_str())) ZipFile << content.c_str();
				content.Clear();
				ZipFile.closeEntry();
			}
		}
	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}








	ZipFile.close();
#endif
};

void	cweeFileSystemLocal::copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeStr exceptFileName) {
	unzipper UnzipFile;
	zipper ZipFile;
	ensureDirectoryExists(DestinationZipFilePath);

	int index = fileLocks.FindIndexWithName(DestinationZipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = DestinationZipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}





	if (UnzipFile.open(SourceZipFilePath)) {	
		if (!ZipFile.open(DestinationZipFilePath, true)) ZipFile.open(DestinationZipFilePath); // try both ways

		for (auto& fileName : UnzipFile.getFilenames()) {
			if (!exceptFileName.IsEmpty() && cweeStr(fileName.c_str()).Find(exceptFileName) >= 0)
				continue; // do not copy this file 

			if (UnzipFile.openEntry((fileName).c_str())) {
				if (ZipFile.addEntry((fileName).c_str())) 
					ZipFile << UnzipFile.ReadEntry().c_str();
				ZipFile.closeEntry();
			}
			UnzipFile.closeEntry();
		}
	}
	ZipFile.close();
	UnzipFile.close();




	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}







};

void		cweeFileSystemLocal::copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeThreadedList<cweeStr> exceptFileNames) {
	unzipper UnzipFile;
	zipper ZipFile;
	ensureDirectoryExists(DestinationZipFilePath);



	int index = fileLocks.FindIndexWithName(DestinationZipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = DestinationZipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}




	if (UnzipFile.open(SourceZipFilePath)) {
		if (!ZipFile.open(DestinationZipFilePath, true)) ZipFile.open(DestinationZipFilePath); // try both ways

		for (auto& fileName : UnzipFile.getFilenames()) {
			bool pass = false;
			for (auto& x : exceptFileNames) {
				if (!x.IsEmpty() && cweeStr(fileName.c_str()).Find(x) >= 0)
					pass = true;
			}
			if (pass) continue;					

			if (UnzipFile.openEntry((fileName).c_str())) {
				if (ZipFile.addEntry((fileName).c_str()))
					ZipFile << UnzipFile.ReadEntry().c_str();
				ZipFile.closeEntry();
			}
			UnzipFile.closeEntry();
		}
	}
	ZipFile.close();
	UnzipFile.close();




	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}










};

void	cweeFileSystemLocal::writeStrListToZipFile(cweeStr zipFilePath, const cweeThreadedList<cweeStr>& data) {
	ensureDirectoryExists(zipFilePath);

	// step 1. Get a copy of all the previous contents. 
	cweeThreadedList<cweeStr> labels; int i = 0;
	for (auto& x : data) {
		i++;
		labels.Append(cweeStr(i));
	}

	// step 2. add the old content back in to the new zip file, AND the new content simultaneously. 
	{
		zipper ZipFile;



		int index = fileLocks.FindIndexWithName(zipFilePath);
		if (index < 0) {
			fileLocks.Lock();
			index = fileLocks.UnsafeAppend();
			auto fPtr = fileLocks.UnsafeRead(index);
			if (fPtr) {
				fPtr->Name = zipFilePath;
			}
			fileLocks.Unlock();
		}
		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Lock();
			}
			fileLocks.AllowDeletion(index);
		}





		if (ZipFile.open(zipFilePath)) {
			ZipFile.close(); // empty it? 
			ZipFile.open(zipFilePath);
			for (i = 0; i < labels.Num(); i++) {
				if (ZipFile.addEntry(labels[i]))
					ZipFile << data[i].c_str();
				ZipFile.closeEntry();				
			}
		}
		ZipFile.close();




		if (index >= 0) {
			fileLocks.PreventDeletion(index);
			fileLocks.Lock();
			auto fPtr = fileLocks.UnsafeRead(index);
			fileLocks.Unlock();
			if (fPtr) {
				fPtr->Unlock();
			}
			fileLocks.AllowDeletion(index);
		}



	}
}

cweeThreadedList<cweeStr> cweeFileSystemLocal::getFileNamesInZip(cweeStr filePath) {
	cweeThreadedList<cweeStr> out;

	unzipper UnzipFile; int ijk;



	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}




	if (UnzipFile.open(filePath))
	{
		for (auto& x : UnzipFile.getFilenames()) {
			out.Append(x.c_str());
		}
	}
	UnzipFile.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}




	return out;
}

int		cweeFileSystemLocal::getNumFilesInZip(cweeStr zipFilePath) {
	int toReturn = -1;

	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}



	unzFile zip = unzOpen(zipFilePath);
	if (zip) toReturn = unzGetNumFiles(zip);
	unzClose(zip);


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}

	return toReturn;
}

void	cweeFileSystemLocal::addFileToZip(cweeStr zipFilePath, cweeStr fileToAddPath) {
	ensureDirectoryExists(zipFilePath);

	cweeStr fileName = fileToAddPath;
	fileName = fileName.StripPath();

	zipper zipFile;



	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}





	zipFile.open(zipFilePath);
	zipFile.addEntry(fileName);

	std::filebuf fb;
	if (fb.open(fileToAddPath, std::ios::in))
	{
		std::istream is(&fb);
		zipFile << is;
		fb.close();
	}
	zipFile.close();




	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}






}

void	cweeFileSystemLocal::addFilesToZip(cweeStr zipFilePath, cweeStr fileToAddPath, cweeStr fileToAddPath2, cweeStr fileToAddPath3, cweeStr fileToAddPath4, cweeStr fileToAddPath5) {
	ensureDirectoryExists(zipFilePath); 
	zipper zipFile;

	int index = fileLocks.FindIndexWithName(zipFilePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = zipFilePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}




	zipFile.open(zipFilePath);
	
	if (fileToAddPath.IsEmpty() == false) {
		cweeStr fileName = fileToAddPath;
		fileName = fileName.StripPath();

		zipFile.addEntry(fileName);
		std::filebuf fb;
		if (fb.open(fileToAddPath, std::ios::in))
		{
			std::istream is(&fb);
			zipFile << is;
			fb.close();
		}
	}
	if (fileToAddPath2.IsEmpty() == false) {
		cweeStr fileName = fileToAddPath2;
		fileName = fileName.StripPath();

		zipFile.addEntry(fileName);
		std::filebuf fb;
		if (fb.open(fileToAddPath2, std::ios::in))
		{
			std::istream is(&fb);
			zipFile << is;
			fb.close();
		}
	}

	if (fileToAddPath3.IsEmpty() == false) {
		cweeStr fileName = fileToAddPath3;
		fileName = fileName.StripPath();

		zipFile.addEntry(fileName);
		std::filebuf fb;
		if (fb.open(fileToAddPath3, std::ios::in))
		{
			std::istream is(&fb);
			zipFile << is;
			fb.close();
		}
	}

	if (fileToAddPath4.IsEmpty() == false) {
		cweeStr fileName = fileToAddPath4;
		fileName = fileName.StripPath();

		zipFile.addEntry(fileName);
		std::filebuf fb;
		if (fb.open(fileToAddPath4, std::ios::in))
		{
			std::istream is(&fb);
			zipFile << is;
			fb.close();
		}
	}

	if (fileToAddPath5.IsEmpty() == false) {
		cweeStr fileName = fileToAddPath5;
		fileName = fileName.StripPath();

		zipFile.addEntry(fileName);
		std::filebuf fb;
		if (fb.open(fileToAddPath5, std::ios::in))
		{
			std::istream is(&fb);
			zipFile << is;
			fb.close();
		}
	}

	zipFile.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}








}





cweeThreadedList<cweeStr>	cweeFileSystemLocal::readFileAsStrList(cweeStr filePath) {
	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);



	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	{
		std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		while (file.good()) {
			getline(file, get);
			if (!cweeStr(get.c_str()).IsEmpty()) {
				size++;
			}
		}
		file.close();

		collection.Resize(size);
		collection.SetNum(size);
		size = 0;
		std::ifstream file2(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		while (file2.good()) {
			getline(file2, get);
			if (!cweeStr(get.c_str()).IsEmpty()) {
				if (size >= collection.Num()) {
					collection.Append(cweeStr(get.c_str()));
				}
				else {
					collection[size] = cweeStr(get.c_str());
				}
				size++;
			}
		}
		collection.SetNum(size);
		file2.close();


	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}






	return collection;
}

void	cweeFileSystemLocal::readFileAsCweeStr(cweeStr& out, cweeStr filePath) {
	out.Clear();
	std::string get;

	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}


	std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (std::getline(file, get)) {
		out.AddToDelimiter(get.c_str(), '\n');
	}
	file.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}


}

void	cweeFileSystemLocal::writeFileFromStrList(cweeStr filePath, cweeThreadedList<cweeStr> const& content) {
	std::fstream fileSave;
	ensureDirectoryExists(filePath);
	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}



	fileSave.open(filePath, std::fstream::out);
	int i = 0;
	for (i = 0; i < content.Num(); i++) {
		if (fileSave.good()) {
			fileSave << content[i] << std::endl;
		}
	}
	fileSave.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}


}

void	cweeFileSystemLocal::writeFileFromCweeStr(cweeStr filePath, cweeStr const& content) {
	std::fstream fileSave;
	ensureDirectoryExists(filePath);

	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}




	fileSave.open(filePath, std::fstream::out);	

	/*
	save entire cweeStr simultaneously
	*/
	if (fileSave.good()) {
		fileSave << content;
	}
	fileSave.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}


}

void	cweeFileSystemLocal::writeFileFromCweeStr(cweeStr folderPath, cweeStr fileName, fileType_t fileType, cweeStr const& content) {

	cweeStr filePath = createFilePath(folderPath, fileName, fileType);
	std::fstream fileSave;


	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}




	fileSave.open(filePath, std::fstream::out);
	/*
	save entire cweeStr simultaneously
	*/
	if (fileSave.good()) {
		fileSave << content;
	}
	fileSave.close();



	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}



}

cweeStr	cweeFileSystemLocal::getApplicationPath(void) {
	wchar_t wtext[MAX_ZIPPED_FILE_NAME];
	LPWSTR buffer(wtext);
	GetModuleFileNameW(0, buffer, MAX_ZIPPED_FILE_NAME);
	std::wstring tempStr(buffer);
	std::string FileDir(std::string(tempStr.begin(), tempStr.end()).erase(std::string(tempStr.begin(), tempStr.end()).length()));
	cweeStr appPath;
	cweeStr(FileDir.c_str()).ExtractFilePath(appPath);
	return appPath;
}

cweeStr	cweeFileSystemLocal::getApplicationName(void) {
	wchar_t wtext[MAX_ZIPPED_FILE_NAME];
	LPWSTR buffer(wtext);
	GetModuleFileNameW(0, buffer, MAX_ZIPPED_FILE_NAME);
	std::wstring tempStr(buffer);
	std::string FileDir(std::string(tempStr.begin(), tempStr.end()).erase(std::string(tempStr.begin(), tempStr.end()).length()));
	cweeStr appPath;
	cweeStr(FileDir.c_str()).ExtractFileBase(appPath);
	return appPath;
}

int	cweeFileSystemLocal::listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr> &list) {
	cweeStr exten = getExtension(extension);
	exten = exten.Right(exten.Length() - 1);
	chaiscript::small_vector<std::string> in = Sys_ListFiles(directory);
	list.Clear();
	for (auto& x : in) {
		cweeStr temp = cweeStr(x.c_str());
		cweeStr extenI; temp.ExtractFileExtension(extenI);
		if (extenI.Icmp(exten) == false || exten == "*")	list.Append(cweeStr(x.c_str()));
	}
	return in.size();
}

chaiscript::small_vector<std::string> cweeFileSystemLocal::listFilesWithExtension(const char* directory, const char* extension) {
	chaiscript::small_vector<std::string> list;
	cweeStr exten = cweeStr(extension);
	exten = exten.Right(exten.Length() - 1);
	chaiscript::small_vector<std::string> in = Sys_ListFiles(directory);
	for (auto& x : in) {
		cweeStr temp = cweeStr(x.c_str());
		cweeStr extenI; temp.ExtractFileExtension(extenI);
		if (extenI.Icmp(exten) == false || exten == "*" || exten.IsEmpty() == true)	list.push_back(x);
	}
	return list;
}

void		cweeFileSystemLocal::saveWindowsPassword(cweeStr account, cweeStr username, cweeStr password) {

	cweeStr Init = account;		char* acc = (char*)Init.c_str();	// trick I like to use to convert cweeStr to valid char*.
	cweeStr Init2 = username;	char* user = (char*)Init2.c_str();	// trick I like to use to convert cweeStr to valid char*.
	cweeStr Init3 = password;	char* pass = (char*)Init3.c_str();	// trick I like to use to convert cweeStr to valid char*.
	
	DWORD cbCreds = 1 + strlen(pass);
	CREDENTIAL cred = { 0 };
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = acc;
	cred.CredentialBlobSize = cbCreds;
	cred.CredentialBlob = (LPBYTE)pass;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.UserName = user;
	BOOL ok = ::CredWrite((&cred), 0);
};
cweeStr		cweeFileSystemLocal::retrieveWindowsPassword(cweeStr account, cweeStr username) {
	cweeStr Init = account;
	char* acc = (char*)Init.c_str();
	cweeStr Init2 = username;
	char* user = (char*)Init2.c_str();
	
	PCREDENTIAL pcred;
	BOOL ok = ::CredRead(acc, CRED_TYPE_GENERIC, 0, &pcred);
	if (!ok) {
		return "";
	}
	else {
		cweeStr out = cweeStr((char*)pcred->CredentialBlob); 
		::CredFree(pcred); // free memory				
		return out;
	}
};

cweeStr		cweeFileSystemLocal::appendLog(cweeStr filePath, cweeStr newLine) {
	ensureDirectoryExists(filePath);
	cweeStr log;
	readFileAsCweeStr(log, filePath);
	log += "\n";
	log += newLine;	
	writeFileFromCweeStr(filePath, log);
	return log;
};

void		cweeFileSystemLocal::copyFile(cweeStr filePathOrig, cweeStr filePathNew) {	
	ensureDirectoryExists(filePathNew);
	int index = fileLocks.FindIndexWithName(filePathOrig);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePathOrig;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}


	CopyFile(filePathOrig.c_str(), filePathNew.c_str(), FALSE);


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}

};

bool		cweeFileSystemLocal::checkFileExists(cweeStr filePath) {
	bool exists = false;

	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}


	std::ifstream f;
	f.open(filePath);
	if (f.good()) {
		exists = true;
	}
	f.close();


	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}


	return exists;
};

int			cweeFileSystemLocal::submitToast(cweeStr title, cweeStr content) {
	int out;
		
	std::pair<cweeStr, cweeStr> toast;	toast.first = title;	toast.second = content;

	Toasts.Lock();
	out = Toasts.UnsafeAppend();
	auto ptr = Toasts.UnsafeRead(out);
	if (ptr) *ptr = toast;
	Toasts.Unlock();
	
	return out;
};

cweeThreadedList< std::pair<int, std::pair<cweeStr, cweeStr>>> cweeFileSystemLocal::getToasts() {
	cweeThreadedList< std::pair<int, std::pair<cweeStr, cweeStr>>> out(Toasts.Num() + 16);

	auto list = Toasts.GetList();

	for (auto& x : list) {
		Toasts.Lock();
		auto ptr = Toasts.UnsafeRead(x);
		if (ptr) {
			out.Append(
				std::make_pair(
					x,
					*ptr
				)
			);
			Toasts.UnsafeErase(x);
		}
		Toasts.Unlock();		

	}

	return out;
};

void		cweeFileSystemLocal::removeToast(int id) {
	// Toasts.Erase(id);
};


#pragma region Comma Seperated Values
INLINE std::map<std::string, cweeThreadedList<cweeStr>> cweeFileSystemLocal::readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders) {
	std::map<std::string, cweeThreadedList<cweeStr>> out;

	cweeThreadedList<std::string> columnToHeader;
	cweeThreadedList<cweeThreadedList<cweeStr>> parsedContent;
	cweeParser parser;

	std::string get;	
	int size(0);

	int index = fileLocks.FindIndexWithName(filePath);
	if (index < 0) {
		fileLocks.Lock();
		index = fileLocks.UnsafeAppend();
		auto fPtr = fileLocks.UnsafeRead(index);
		if (fPtr) {
			fPtr->Name = filePath;
		}
		fileLocks.Unlock();
	}
	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Lock();
		}
		fileLocks.AllowDeletion(index);
	}

	{
		std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		while (file.good()) {
			getline(file, get);
			if (!cweeStr(get.c_str()).IsEmpty()) {
				size++;
			}
		}
		file.close();

		
		std::ifstream file2(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		if (myDataHasHeaders && file2.good()) {
			// get names from first row
			getline(file2, get);
			if (!get.empty()) {
				parser.Parse(get.c_str(), ",", true);
				for (auto& x : parser) {
					columnToHeader.Append(x.c_str());
					parsedContent.Append(cweeThreadedList<cweeStr>(size + 16));
				}
			}			
		}

		while (file2.good()) {
			getline(file2, get);
			if (!get.empty()) {
				parser.Parse(get.c_str(), ",", true);
				if (columnToHeader.Num() == 0 && (parser.getNumVars() > columnToHeader.Num())) {
					for (int i = columnToHeader.Num(); i < parser.getNumVars(); i++) {
						columnToHeader.Append(cweeStr::ToString(i).c_str());
						parsedContent.Append(cweeThreadedList<cweeStr>(size+16));
					}
				}
				for (int i = cweeMath::min(columnToHeader.Num(), parser.getNumVars()) - 1; i >= 0; i--) {
					parsedContent[i].Append(parser[i]);
				}
			}
		}

		file2.close();


	}

	if (index >= 0) {
		fileLocks.PreventDeletion(index);
		fileLocks.Lock();
		auto fPtr = fileLocks.UnsafeRead(index);
		fileLocks.Unlock();
		if (fPtr) {
			fPtr->Unlock();
		}
		fileLocks.AllowDeletion(index);
	}

	for (int i = cweeMath::min(columnToHeader.Num(), parsedContent.Num()) - 1; i >= 0; i--) {
		out[columnToHeader[i]].Swap(parsedContent[i]);
	}

	return out;
};
#pragma endregion

INLINE void	DoRouterRequest(cweeRouterRequest* io) {
	int											request_index = -1;
	cweeUnorderedList< cweeRouterRequest >*		routerRequests = nullptr;
	
	if (io && io->routerRequests) {
		routerRequests = io->routerRequests;
		
		cweeStr request;		
		io->Lock(); {
			io->started = true;
			request = io->request;
		} io->Unlock();

		auto job = scripting->Do(request);
		job.ContinueWith([=]() { // job, io, request_index, routerRequests
			cweeStr& reply = job.GetResult().cast();
			io->Lock(); {
				io->completed = true;
				io->reply = reply;
			} io->Unlock();
			fileSystem->ReplyToRequestClient(io); // in-thread job

			if (routerRequests) {
				routerRequests->Erase(request_index);
			}
		});
	}
	else {
		if (routerRequests) {
			routerRequests->Erase(request_index);
		}
	}
};
REGISTER_WITH_PARALLEL_THREAD(DoRouterRequest);