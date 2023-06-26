

#ifndef	__FILESYSTEM_H__
#define __FILESYSTEM_H__



typedef enum {
	ANY_EXT = -1,
	INP = 0,
	TXT = BIT(1),
	CSV = BIT(2),
	EXL = BIT(3),
	DAT = BIT(4),
	ZIP = BIT(5),
	ZIP_7z = BIT(6),
	sqlDB = BIT(7),
	EDMS = BIT(8),
	PK4 = BIT(9),
	PROJECT = BIT(10),
	PATFILE = BIT(11)
} fileType_t;

typedef enum {
	FS_READ = 0,
	FS_WRITE = 1,
	FS_APPEND = 2
} fsMode_t;

class IpAddressInformation {
public:
	cweeStr city;
	cweeStr country_code;
	cweeStr country_name;
	cweeStr ip;
	cweeStr latitude;
	cweeStr longitude;
	cweeStr metro_code;
	cweeStr region_code;
	cweeStr region_name;
	cweeStr time_zone;
	cweeStr zip_code;
	vec3d	coordinates; // longitude, latitude, elevation
};

class cweeFileSystem {
public:
	virtual	void		Init(void) = 0;
	virtual void		resetReadCount(void) = 0;
	virtual void		addToReadCount(int c) = 0;
	virtual int			getReadCount(int c) = 0;
	
	virtual cweeStr		getAppFolder(void) = 0;
	virtual cweeStr		getAppName(void) = 0;
	virtual cweeStr		getDataFolder(void) = 0;
	virtual void		setDataFolder(cweeStr in) = 0;

	virtual void		removeFile(const cweeStr& filePath) = 0;
	virtual void		renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath) = 0;

	virtual bool		ensureDirectoryExists(const cweeStr& Directory) = 0;
	virtual cweeStr		createFilePath(cweeStr directory, cweeStr fileName, fileType_t fileType) = 0;
	virtual cweeThreadedList<cweeStr> readFileAsStrList(cweeStr filePath) = 0;
	virtual void		readFileAsCweeStr(cweeStr& out, cweeStr filePath) = 0;
	virtual void		writeFileFromStrList(cweeStr filePath, cweeThreadedList<cweeStr> const& content) = 0;
	virtual void		writeFileFromCweeStr(cweeStr filePath, cweeStr const& content) = 0;

	virtual void		writeFileFromCweeStr(cweeStr folderPath, cweeStr fileName, fileType_t fileType, cweeStr const& content) = 0;

	virtual void		setCurrentTime(const u64& time, bool* mode = nullptr) = 0;
	virtual u64			getCurrentTime() = 0;
	virtual cweeTime	localtime(const u64& time) = 0;
	virtual cweeStr		createTimeFromMinutes(float minutes) = 0;
	virtual time_t		getFirstDayOfSameMonth(time_t in) = 0;
	virtual time_t		getLastDayOfSameMonth(time_t in) = 0;
	virtual int			getNumDaysInSameMonth(time_t in) = 0;

	//virtual cweeStr		USPS_FixAddress(const cweeStr& address) = 0;
	virtual std::map<std::string, cweeStr> Geocode_Census(const cweeStr& address) = 0;
	virtual cweeStr		DownloadCweeStrFromURL(cweeStr url) = 0;
	virtual HRESULT		downloadFileFromURL(cweeStr url, cweeStr destinationFilePath) = 0;
	virtual cweeStr		ReadFirstFileFromZipToString(cweeStr zipFilePath) = 0;
	virtual cweeStr		ReadFileFromZipToString(cweeStr zipFilePath, int whichStartingZero)=0;
	virtual cweeStr		ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName) = 0;	
	virtual void		ReadFileFromZipToString(cweeStr zipFilePath, cweeStr fileName, cweeStr& out) = 0;
	virtual void		addContentToZip(cweeStr zipFilePath, const cweeStr& fileName, cweeStr& content) = 0;
	virtual void		copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeStr exceptFileName = "") = 0;
	virtual void		copyContentBetweenZips(cweeStr DestinationZipFilePath, cweeStr SourceZipFilePath, cweeThreadedList<cweeStr> exceptFileNames) = 0;
	virtual void		writeStrListToZipFile(cweeStr zipFilePath, const cweeThreadedList<cweeStr>& content) = 0;

	virtual cweeThreadedList<cweeStr> getFileNamesInZip(cweeStr filePath) = 0;
	virtual int			getNumFilesInZip(cweeStr zipFilePath) = 0; 
	virtual void		addFileToZip(cweeStr zipFilePath, cweeStr fileToAddPath) = 0;
	virtual void		addFilesToZip(cweeStr zipFilePath, cweeStr fileToAddPath, cweeStr fileToAddPath2 = "", cweeStr fileToAddPath3 = "", cweeStr fileToAddPath4 = "", cweeStr fileToAddPath5 = "") = 0;

	virtual u64			returnTime_gm(int year, int month, int day, int hour, int minute, int second) = 0;
	virtual u64			returnTime(int year, int month, int day, int hour, int minute, int second) = 0;
	virtual int			getByteSizeOfFile(cweeStr filePath) = 0;
	virtual int			listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list) = 0;
	virtual std::vector<std::string> listFilesWithExtension(const char* directory, const char* extension) = 0;
	
	virtual void		saveWindowsPassword(cweeStr account, cweeStr username, cweeStr password) = 0;
	virtual cweeStr		retrieveWindowsPassword(cweeStr account, cweeStr username) = 0;

	virtual cweeStr		appendLog(cweeStr filePath, cweeStr newLine) = 0;
	virtual void		copyFile(cweeStr filePathOrig, cweeStr filePathNew) = 0;
	virtual bool		checkFileExists(cweeStr filePath) = 0;
	virtual int			submitToast(cweeStr title, cweeStr content) = 0;
	virtual cweeThreadedList< std::pair<int, std::pair<cweeStr, cweeStr>>> getToasts() = 0;
	virtual void		removeToast(int id) = 0;

	virtual cweeStr		getExtension(fileType_t fileType) = 0;

#pragma region Comma Seperated Values
	virtual std::map<std::string, cweeThreadedList<cweeStr>> readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders = true) = 0;
	virtual std::map<int, cweeThreadedList<std::pair<u64, float>>> readSCADA(const cweeStr& filePath, bool myDataHasHeaders = true) = 0;
#pragma endregion





#pragma region "Async Publish/Subscribe Model (N-Subscribers to 1-Publisher) (Listen Only)"
	/*! Start a server that publishes "messages" to listeners that "subscribe" to the port on this machine. There can only be one publisher per port. */
	virtual bool		StartPublishingServer(int Port = 5556) = 0;
	/*! End a publishing server at the specified port on this machine. */
	virtual bool		ClosePublishingServer(int Port = 5556) = 0;
	/*! Return a list of all ports used by this app on this machine for publishing. */
	virtual cweeThreadedList<int>	GetPublishingServerPorts() = 0;
	/*! Send a message out through the specified port to all current subscribers. */
	virtual bool		UpdatePublishingServer(const cweeStr& message, int Port = 5556) = 0;

	/*! Start a subscriber on this machine that listens for updates from a publisher at the specified address/port. There can only be one subscriber per machine to any specific address/port combo. */
	virtual bool		StartSubscriptionClient(const cweeStr& address = "localhost", int Port = 5556) = 0;
	/*! End a subscriber on this machine listening to the specified address/host. */
	virtual bool		CloseSubscriptionClient(const cweeStr& address = "localhost", int Port = 5556) = 0;
	/*! Return a list of all addresses/ports being listened to by this app on this machine. */
	virtual cweeThreadedList<std::pair<cweeStr, int>> GetSubscriptionClients() = 0;
	/*! 
	Check for any updates from the subscribed publishers. First call after subscribing will always fail. Repeat/frequent calls are highly recommended. 
	Allowed to "miss" published messages, as the computer can buffer not-yet-grabbed messages. 
	Will only return one message at a time. If the publisher sent out 1000 messages - this must then be called 1000 times to recieve them all. 
	*/
	virtual bool		TryGetSubscriptionUpdate(cweeStr& out, const cweeStr& address = "localhost", int Port = 5556) = 0;
#pragma endregion

#pragma region "Async Client/Router Model (N-Clients to 1-Router) (Ask-Reply Only)"
	virtual bool		StartRouterServer(int Port = 5556) = 0;
	virtual bool		CloseRouterServer(int Port = 5556) = 0;
	virtual cweeThreadedList<int>	GetRouterServerPorts() = 0;
	virtual bool		UpdateRouterServer(int Port = 5556) = 0;
	virtual bool		ReplyToRequestClient(void* RouterRequestData) = 0;

	virtual bool		StartRequestClient(const cweeStr& address = "localhost", int Port = 5556) = 0;
	virtual bool		CloseRequestClient(const cweeStr& address = "localhost", int Port = 5556) = 0;
	virtual cweeThreadedList<std::pair<cweeStr, int>> GetRequestClients() = 0;
	virtual bool		TrySendRequest(const cweeStr& request, const cweeStr& address = "localhost", int Port = 5556) = 0;
	virtual bool		TryGetReply(cweeStr& out, const cweeStr& address = "localhost", int Port = 5556) = 0;
	virtual cweeStr		GetReply(const cweeStr& request, const cweeStr& address = "localhost", int Port = 5556) = 0;

#pragma endregion

#pragma region "HTTP Request Functions"
	virtual cweeThreadedList<std::pair<cweeStr, cweeStr>> ParseJson(cweeStr source, bool* failed = nullptr) = 0;
	virtual IpAddressInformation GetIpAddressInformation() = 0;


	virtual cweeThreadedList<std::pair<u64, float>> GetWeatherData(const u64& start, const u64& end, const double& longitude, const double& latitude) = 0;
	virtual cweeStr QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "EdmsApp") = 0;
#pragma endregion

	virtual void		Pack_Message(cweeStr& toPackage) = 0;
	virtual void		Unpack_Message(cweeStr& toUnpackage) = 0;
};

extern cweeFileSystem* fileSystem;

struct readFileAsStrList_ParallelJobPackage {
	cweeStr filePath = cweeStr("");
	cweeThreadedList<cweeStr>* fileContent;
};
INLINE void	readFileAsStrList(readFileAsStrList_ParallelJobPackage* io) {

	std::string get;
	io->fileContent->SetNum(0);
	int size(0);
	std::ifstream file(io->filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file.good()) {
		getline(file, get);
		if (!cweeStr(get.c_str()).IsEmpty()) {
			size++;
		}
	}
	file.close();

	io->fileContent->Resize(size);
	io->fileContent->SetNum(size);
	size = 0;
	std::ifstream file2(io->filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
	while (file2.good()) {
		getline(file2, get);
		if (!cweeStr(get.c_str()).IsEmpty()) {
			if (size >= io->fileContent->Num()) {
				io->fileContent->Append(cweeStr(get.c_str()));
			}
			else {
				io->fileContent->operator[](size) = cweeStr(get.c_str());
			}
			size++;
		}
	}
	io->fileContent->SetNum(size);
	file2.close();

	delete io; // fileContent is on the heap so just go read from there.
}











#endif