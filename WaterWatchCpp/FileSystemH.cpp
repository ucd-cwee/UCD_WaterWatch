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
#include "Units.h"
#include "../cNominitum/cNominitum.h"
#include "Geocoding.h"
#include "FileSystemH.h"
#include "InterlockedValues.h"
#include "cweeInterlocked.h"
#include "DispatchTimer.h"
#include "cweeThreadedMap.h"
#include "Mutex.h"
#include "Toasts.h"
#include "chaiscript_wrapper.h"

#define CURL_STATICLIB
#include "../CURL_STATIC_LIB/CURL_STATIC_LIB.h"


class FileSystemLocal : public FileSystem {
private:
	static constexpr long								MAX_ZIPPED_FILE_NAME = 2048;
	static constexpr long								FILE_HASH_SIZE = 1024;
	static constexpr auto								privateServerKey = "yevUcBl^KyeCO*WUqF6}n:Ci/RzcmqLiT^#>IZ!f"; // public should be == jow0MY5igjre>]VY*P=5COS0$1Ztk]3<R<1MgrPT
	static constexpr long								currentRealTimeAccuracyMilliseconds = 100;
	cweeSharedPtr<cweeStr>								ip_address;
	cweeSharedPtr< IpAddressInformation >				ip_address_info;
	const u64											timeZoneAdjust;
	const cweeTime										epoch; // 1970/1/1 00:00:00
	const cweeStr 										appFolder;
	const cweeStr										appName;
	cweeUnpooledInterlocked < cweeStr >					dataFolder;
	cweeSharedPtr < cweeTime >							currentRealTime; // updated automatically by currentRealTimeUpdater.
	DispatchTimer										currentRealTimeUpdater; // updates within the accuracy of 'currentRealTimeAccuracyMilliseconds' ... thread will automatically die with this class instance.
	cweeThreadedMap<cweeStr, cweeSysMutex>				fileLocks;
	cweeThreadedMap < cweeStr, bool>					tempFiles;

public:
	FileSystemLocal();
	~FileSystemLocal();

private:
	void ClearTempFiles();

public: // File Locks
	void LockFile(cweeStr const& filePath);
	void UnlockFile(cweeStr const& filePath);
	cweeSharedPtr<cweeStr> GuardFile(cweeStr const& filePath);

private:
	cweeStr			getUniqueFingerprint();
	void			tryGetIpAddress(int numAttempts = 0);

public:
	void			removeFile(const cweeStr& filePath);
	void			renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath);
	cweeStr			getAppFolder();
	cweeStr			getAppName();
	cweeStr			getDataFolder();
	cweeStr			setDataFolder(cweeStr const& in);
	cweeStr			createFilePath(cweeStr const& directory, cweeStr const& fileName, fileType_t const& fileType);
	cweeStr			createRandomFilePath(fileType_t const& fileType);
	cweeStr			createRandomFile(fileType_t const& fileType);
	cweeStr			createRandomFile(cweeStr const& fileType);
	u64				getCurrentTime();

	cweeStr			QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "WaterWatchCpp");
	cweeStr			GetIpAddress();
	IpAddressInformation			GetAddress();

public:
	cweeThreadedList<cweeStr>	readFileAsStrList(cweeStr const& filePath);
	void	readFileAsCweeStr(cweeStr& out, cweeStr const& filePath);
	void	writeFileFromStrList(cweeStr const& filePath, cweeThreadedList<cweeStr> const& content);
	void	writeFileFromCweeStr(cweeStr const& filePath, cweeStr const& content);
	void	writeFileFromCweeStr(cweeStr const& folderPath, cweeStr const& fileName, fileType_t const& fileType, cweeStr const& content);

	void	downloadFileFromURL(cweeStr const& url, cweeStr const& destinationFilePath);
	cweeStr DownloadCweeStrFromURL(cweeStr const& url);

public:
	int		getByteSizeOfFile(cweeStr const& filePath);
	cweeStr ReadFirstFileFromZipToString(cweeStr const& zipFilePath);
	cweeStr ReadFileFromZipToString(cweeStr const& zipFilePath, int whichStartingZero);
	cweeStr	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName);
	void	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName, cweeStr& out);
	void	addContentToZip(cweeStr const& zipFilePath, const cweeStr& fileName, cweeStr& content);
	void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeStr const& exceptFileName);
	void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeThreadedList<cweeStr> const& exceptFileNames);
	void	writeStrListToZipFile(cweeStr const& zipFilePath, const cweeThreadedList<cweeStr>& data);
	cweeList<cweeStr> getFileNamesInZip(cweeStr const& filePath);
	int		getNumFilesInZip(cweeStr const& zipFilePath);
	void	addFileToZip(cweeStr const& zipFilePath, cweeStr const& fileToAddPath);

public:
	int		listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list);
	cweeList<cweeStr> listFilesWithExtension(const char* directory, const char* extension);
	cweeList<cweeStr> listFilesWithExtension(const char* directory, fileType_t extension);
public:
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	void		saveWindowsPassword(cweeStr const& account, cweeStr const& username, cweeStr  const& password);
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	cweeStr		retrieveWindowsPassword(cweeStr const& account, cweeStr const& username);
	cweeStr		appendLog(cweeStr const& filePath, cweeStr const& newLine);
	void		copyFile(cweeStr const& filePathOrig, cweeStr const& filePathNew);
	bool		checkFileExists(cweeStr const& filePath);

	void		submitToast(cweeStr const& title, cweeStr const& content);
	cweeThreadedList< std::pair<cweeStr, cweeStr>> getToasts();

public:
	std::map<std::string, cweeThreadedList<cweeStr>> readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders = true);
	std::map<int, cweeThreadedList<std::pair<u64, float>>> readSCADA(const cweeStr& filePath, bool myDataHasHeaders = true);

};

FileSystemLocal::FileSystemLocal() : FileSystem(), 
	timeZoneAdjust((boost::posix_time::microsec_clock::local_time() - boost::posix_time::microsec_clock::universal_time()).total_milliseconds() / 1000.0)
	, epoch(boost::posix_time::time_from_string("1970/1/1 0:0:0"))
	, appFolder(getApplicationPath())
	, appName(getApplicationName())
	, dataFolder(setDataFolder(appFolder + cweeStr("data\\")))
	, currentRealTime(new cweeTime())
	, currentRealTimeUpdater(currentRealTimeAccuracyMilliseconds, cweeJob([](cweeSharedPtr<cweeTime> ptr) { u64 t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0; AUTO g = ptr.Guard(); *ptr.UnsafeGet() = t; }, currentRealTime))
	, fileLocks()
	, ip_address(make_cwee_shared<cweeStr>())
	, ip_address_info(make_cwee_shared<IpAddressInformation>())
{
	tryGetIpAddress();
};

void FileSystemLocal::tryGetIpAddress(int numAttempts) {
	if (numAttempts > 100) return;
	cweeJob([this](cweeSharedPtr<cweeStr> Addr, int NumAttempts, cweeSharedPtr< IpAddressInformation> AddrInfo) {
		if (Addr) {
			cweeStr query = this->QueryHttp("api.ipify.org", "");
			if (query != "") {
				AUTO g = Addr.Guard();
				Addr.UnsafeGet()->operator=(query);

				{
					AUTO destinationFilePath = this->createRandomFile(fileType_t::TXT);

					// LETS DO THE CURL OPERATION
					cweeStr read;
					{
						cweeCURL curl;
						this->LockFile(destinationFilePath);
						{
							int result;
							FILE* fp = fopen(destinationFilePath, "wb");
							result = curl.setopt(cweeCURL::CURLoption::CURLOPT_CUSTOMREQUEST, "GET");
							result = curl.setopt(cweeCURL::CURLoption::CURLOPT_URL, "https://find-any-ip-address-or-domain-location-world-wide.p.rapidapi.com/iplocation?apikey=873dbe322aea47f89dcf729dcc8f60e8");
							curl.setopt(cweeCURL::CURLoption::CURLOPT_WRITEFUNCTION, &fileSys_write_data);
							curl.setopt(cweeCURL::CURLoption::CURLOPT_WRITEDATA, fp);
							cweeThreadedList<cweeStr> headers;
							headers.Append("X-RapidAPI-Key: 5ad46c78bbmsh509c23a7b43a6d5p1e1963jsn4998d1523143");
							headers.Append("X-RapidAPI-Host: find-any-ip-address-or-domain-location-world-wide.p.rapidapi.com");
							result = curl.setopt(cweeCURL::CURLoption::CURLOPT_HTTPHEADER, headers);
							result = curl.perform();
							fclose(fp);
						}
						this->UnlockFile(destinationFilePath);
					}

					readFileAsCweeStr(read, destinationFilePath);
					removeFile(destinationFilePath);
					read.ReplaceInline("\\", "/");

					chaiscript::ChaiScript engine;
					AUTO Bv = chaiscript::const_var(std::string(read.c_str()));
					engine.add_global_const(Bv, "srce");
					try {
						engine.eval("global& m = srce.from_json();");

						AddrInfo->country_name = engine.eval<std::string>("return m[\"country\"].to_string()").c_str();
						AddrInfo->region_name = engine.eval<std::string>("return m[\"state\"].to_string()").c_str();
						AddrInfo->city = engine.eval<std::string>("return m[\"city\"].to_string()").c_str();

						AddrInfo->ip = engine.eval<std::string>("return m[\"ip\"].to_string()").c_str();
						AddrInfo->latitude = engine.eval<std::string>("return m[\"latitude\"].to_string()").c_str();
						AddrInfo->longitude = engine.eval<std::string>("return m[\"longitude\"].to_string()").c_str();

						AddrInfo->time_zone = engine.eval<std::string>("return m[\"timezone\"].to_string()").c_str();
						AddrInfo->zip_code = engine.eval<std::string>("return m[\"zipCode\"].to_string()").c_str();
					}
					catch (...) {}
					AddrInfo->coordinates = vec3d(AddrInfo->longitude.ReturnNumericD(), AddrInfo->latitude.ReturnNumericD(), geocoding->GetElevation(vec2d(AddrInfo->longitude.ReturnNumericD(), AddrInfo->latitude.ReturnNumericD())));
				}
			}
			else {
				this->tryGetIpAddress(NumAttempts + 1);
			}
		}
	}, ip_address, numAttempts, ip_address_info).DelayedAsyncForceInvoke(1000);
};

FileSystemLocal::~FileSystemLocal() {
	ClearTempFiles();
};
void FileSystemLocal::ClearTempFiles() {
	for (auto& f : tempFiles) {
		removeFile(f.first);
	}
	tempFiles.Clear();
};

void FileSystemLocal::LockFile(cweeStr const& filePath) {
	fileLocks[filePath]->Lock(); // if the directory does not already exist, it will shortly.
};
void FileSystemLocal::UnlockFile(cweeStr const& filePath) {
	fileLocks[filePath]->Unlock();
};
cweeSharedPtr<cweeStr> FileSystemLocal::GuardFile(cweeStr const& filePath) {
	LockFile(filePath);
	return cweeSharedPtr<cweeStr>(new cweeStr(filePath), [this](cweeStr* p) { this->UnlockFile(*p); delete p; });
};


cweeStr			FileSystemLocal::getUniqueFingerprint() {
	cweeStr MAC_ADDRESS;
	cweeStr IP_ADDRESS;
	{
#if 0
		PIP_ADAPTER_INFO AdapterInfo;
		DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

		AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
		if (AdapterInfo != NULL) {
			if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
				free(AdapterInfo);
				AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
			}
			if (AdapterInfo != NULL) {
				if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
					PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
					do {
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
#endif
	}
	return MAC_ADDRESS + "-" + IP_ADDRESS;// +"-" + randomID;
};

void			FileSystemLocal::removeFile(const cweeStr& filePath) {
	LockFile(filePath);
	std::remove(filePath);
	UnlockFile(filePath);
};
void			FileSystemLocal::renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath) {
	LockFile(oldFilePath);
	auto _ = std::rename(oldFilePath, newFilePath);
	UnlockFile(oldFilePath);
};
cweeStr			FileSystemLocal::getAppFolder() { return appFolder; };
cweeStr			FileSystemLocal::getAppName() { return appName; };
cweeStr			FileSystemLocal::getDataFolder() { return dataFolder; };
cweeStr			FileSystemLocal::setDataFolder(cweeStr const& in) {
	ensureDirectoryExists(in);
	dataFolder = in;
	return in;
};
cweeStr			FileSystemLocal::createFilePath(cweeStr const& directory, cweeStr const& fileName, fileType_t const& fileType) {
	ensureDirectoryExists(directory);
	return directory + "\\" + fileName + getExtension(fileType);
};
cweeStr			FileSystemLocal::createRandomFilePath(fileType_t const& fileType) {
	return createFilePath(getDataFolder() + "\\temp", cweeStr::printf("%i_%i_%i", cweeRandomInt(0, 1000), cweeRandomInt(0, 1000), cweeRandomInt(0, 1000)), fileType);
};
cweeStr			FileSystemLocal::createRandomFile(fileType_t const& fileType) {
	cweeStr filePath = createFilePath(getDataFolder() + "\\temp", cweeStr::printf("%i_%i_%i", cweeRandomInt(0, 1000), cweeRandomInt(0, 1000), cweeRandomInt(0, 1000)), fileType);
	this->writeFileFromCweeStr(filePath, "");
	tempFiles.Emplace(filePath, false);
	return filePath;
};
cweeStr			FileSystemLocal::createRandomFile(cweeStr const& fileType) {
	ensureDirectoryExists(getDataFolder() + "\\temp");
	cweeStr filePath = 
		getDataFolder() + 
		"\\temp\\" + 
		cweeStr::printf("%i_%i_%i", cweeRandomInt(0, 1000), cweeRandomInt(0, 1000), cweeRandomInt(0, 1000)) + 
		fileType;

	this->writeFileFromCweeStr(filePath, "");
	tempFiles.Emplace(filePath, false);
	return filePath;
};
u64				FileSystemLocal::getCurrentTime() {
	u64 out(0);
	AUTO g = currentRealTime.Guard();
	out = currentRealTime.UnsafeGet()->operator u64();
	return out;
};

cweeStr  FileSystemLocal::QueryHttp(const cweeStr& mainAddress, const cweeStr& requestParameters, const cweeStr& UniqueSessionName) {
	cweeStr filePath, out;

	C_WINHTTP http;

	HRESULT hr = http.Query(filePath, mainAddress, requestParameters, UniqueSessionName);	

	{
		std::string get;
		std::ifstream file(filePath.c_str()); 
		out.Clear();
		while (std::getline(file, get)) {
			out.AddToDelimiter(get.c_str(), '\n');
		}
		file.close();
	}
	std::remove(filePath.c_str());

	return out;
};
cweeStr			FileSystemLocal::GetIpAddress() {
	cweeStr out;
	if (ip_address) {
		AUTO g = ip_address.Guard();
		out = *ip_address.UnsafeGet();
	}
	return out;
};

IpAddressInformation			FileSystemLocal::GetAddress() {
	IpAddressInformation out;
	if (ip_address_info) {
		AUTO g = ip_address_info.Guard();
		out = *ip_address_info.UnsafeGet();
	}
	return out;
};

cweeThreadedList<cweeStr>	FileSystemLocal::readFileAsStrList(cweeStr const& filePath) {
	// pre-read to determine size of file to pre-load memory
	cweeThreadedList<cweeStr> collection;
	std::string get;
	collection.SetNum(0);
	int size(0);
	LockFile(filePath);
	{
		std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		while (file.good()) {
			getline(file, get);
			if (!get.empty()) {
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
			if (!get.empty()) {
				if (size >= collection.Num()) {
					collection.Alloc() == get.c_str();
				}
				else {
					collection[size] = get.c_str();
				}
				size++;
			}
		}
		collection.SetNum(size);
		file2.close();
	}
	UnlockFile(filePath);
	return collection;
};
void	FileSystemLocal::readFileAsCweeStr(cweeStr& out, cweeStr const& filePath) {
	out.Clear();
	std::string get;

	LockFile(filePath);
	{
		std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
		while (std::getline(file, get)) {
			out.AddToDelimiter(get.c_str(), '\n');
		}
		file.close();
	}
	UnlockFile(filePath);
};
void	FileSystemLocal::writeFileFromStrList(cweeStr const& filePath, cweeThreadedList<cweeStr> const& content) {
	ensureDirectoryExists(filePath);
	std::fstream fileSave;
	LockFile(filePath);
	{
		fileSave.open(filePath, std::fstream::out);
		int i = 0;
		for (i = 0; i < content.Num(); i++) {
			if (fileSave.good()) {
				fileSave << content[i] << std::endl;
			}
		}
		fileSave.close();
	}
	UnlockFile(filePath);
};
void	FileSystemLocal::writeFileFromCweeStr(cweeStr const& filePath, cweeStr const& content) {
	ensureDirectoryExists(filePath);
	std::fstream fileSave;
	LockFile(filePath);
	{

		fileSave.open(filePath, std::fstream::out);

		/*
		save entire cweeStr simultaneously
		*/
		if (fileSave.good()) {
			fileSave << content;
		}
		fileSave.close();
	}
	UnlockFile(filePath);
};
void	FileSystemLocal::writeFileFromCweeStr(cweeStr const& folderPath, cweeStr const& fileName, fileType_t const& fileType, cweeStr const& content) {
	std::fstream fileSave; cweeStr filePath = createFilePath(folderPath, fileName, fileType);
	LockFile(filePath);
	{
		fileSave.open(filePath, std::fstream::out);
		/*
		save entire cweeStr simultaneously
		*/
		if (fileSave.good()) {
			fileSave << content;
		}
		fileSave.close();
	}
	UnlockFile(filePath);
};

void	FileSystemLocal::downloadFileFromURL(cweeStr const& url, cweeStr const& destinationFilePath) {
	ensureDirectoryExists(destinationFilePath);
	writeFileFromCweeStr(destinationFilePath, QueryHttp(url, ""));
};
cweeStr FileSystemLocal::DownloadCweeStrFromURL(cweeStr const& url) {
	cweeStr content;
	{
		AUTO randPath = createRandomFilePath(fileType_t::TXT);
		downloadFileFromURL(url, randPath);
		readFileAsCweeStr(content, randPath);
		removeFile(randPath);
	}
	return content;
};

int		FileSystemLocal::getByteSizeOfFile(cweeStr const& filePath) {
	std::streampos begin, end;

	LockFile(filePath);
	{ // do something with the file
		std::ifstream file(filePath, std::ios::binary);
		begin = file.tellg();
		file.seekg(0, std::ios::end);
		end = file.tellg();
		file.close();
	}
	UnlockFile(filePath);

	return (end - begin);
};
cweeStr FileSystemLocal::ReadFirstFileFromZipToString(cweeStr const& zipFilePath) {
	cweeStr digitized;
#if 0
	unzipper zipFile;

	AUTO g = GuardFile(zipFilePath);

	zipFile.open(zipFilePath);
	
	int numFiles = 0;
	for (std::string filename : zipFile.getFilenames())
	{
		if (numFiles > 0) break;
		cweeStr tempFile = createRandomFilePath(fileType_t::TXT);

		zipFile.openEntry(filename.c_str());
		std::ofstream wFile;
		wFile.open(tempFile.c_str());
		zipFile >> wFile;
		wFile.close();

		readFileAsCweeStr(digitized, tempFile);
		remove(tempFile.c_str());

		digitized.Replace("\n\n", "\n"); // why is this even a thing
		numFiles++;
	}
	zipFile.close();
#endif
	return digitized;
};
cweeStr FileSystemLocal::ReadFileFromZipToString(cweeStr const& zipFilePath, int whichStartingZero) {
	cweeStr digitized;
#if 0
	unzipper zipFile;

	AUTO g = GuardFile(zipFilePath);

	zipFile.open(zipFilePath);
	
	int numFiles = 0;

	auto list = zipFile.getFilenames();

	std::sort(list.begin(), list.end(), [](std::string const& a, std::string const& b)->bool { return a < b; }); //sort the vector alphebetically

	for (auto& filename : list)
	{
		if (numFiles > whichStartingZero) {
			break;
		}
		{
			if (numFiles == whichStartingZero) {
				zipFile.openEntry(filename.c_str());
				zipFile.ReadEntry(digitized);
				digitized.Replace("\n\n", "\n"); // why is this even a thing
				zipFile.closeEntry();
			}
		
		}
		numFiles++;
	}
	zipFile.close();
#endif
	return digitized;
};
cweeStr	FileSystemLocal::ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName) {
	cweeStr out;
#if 0
	unzipper UnzipFile;

	AUTO g = GuardFile(zipFilePath);

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
#endif
	return out;
};;;
void	FileSystemLocal::ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName, cweeStr& out) {
#if 0
	out.Clear();

	unzipper UnzipFile;

	AUTO g = GuardFile(zipFilePath);

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
#endif
};
void	FileSystemLocal::addContentToZip(cweeStr const& zipFilePath, const cweeStr& fileName, cweeStr& content) {
#if 0
	ensureDirectoryExists(zipFilePath);

	zipper ZipFile;

	AUTO g = GuardFile(zipFilePath);

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

	ZipFile.close();
#endif
};;
void	FileSystemLocal::copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeStr const& exceptFileName) {
#if 0
	if (DestinationZipFilePath == SourceZipFilePath) return;

	unzipper UnzipFile;
	zipper ZipFile;
	ensureDirectoryExists(DestinationZipFilePath);

	AUTO g = GuardFile(DestinationZipFilePath);
	AUTO g2 = GuardFile(SourceZipFilePath);

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
#endif
};
void	FileSystemLocal::copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeThreadedList<cweeStr> const& exceptFileNames) {
#if 0
	unzipper UnzipFile;
	zipper ZipFile;
	ensureDirectoryExists(DestinationZipFilePath);

	AUTO g = GuardFile(DestinationZipFilePath);
	AUTO g2 = GuardFile(SourceZipFilePath);


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
#endif
};;
void	FileSystemLocal::writeStrListToZipFile(cweeStr const& zipFilePath, const cweeThreadedList<cweeStr>& data) {
#if 0
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



		AUTO g = GuardFile(zipFilePath);





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



	}
#endif
};
cweeList<cweeStr> FileSystemLocal::getFileNamesInZip(cweeStr const& filePath) {
	cweeThreadedList<cweeStr> out;
#if 0
	unzipper UnzipFile; int ijk;

	AUTO g = GuardFile(filePath);

	if (UnzipFile.open(filePath))
	{
		for (auto& x : UnzipFile.getFilenames()) {
			out.Append(x.c_str());
		}
	}
	UnzipFile.close();
#endif
	return out;
};
int		FileSystemLocal::getNumFilesInZip(cweeStr const& zipFilePath) {
	int toReturn = -1;
#if 0
	AUTO g = GuardFile(zipFilePath);



	unzFile zip = unzOpen(zipFilePath);
	if (zip) toReturn = unzGetNumFiles(zip);
	unzClose(zip);
#endif
	return toReturn;
};
void	FileSystemLocal::addFileToZip(cweeStr const& zipFilePath, cweeStr const& fileToAddPath) {
#if 0

	ensureDirectoryExists(zipFilePath);

	cweeStr fileName = fileToAddPath;
	fileName = fileName.StripPath();

	zipper zipFile;

	AUTO g = GuardFile(zipFilePath);
	AUTO g2 = GuardFile(fileToAddPath);



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
#endif
};

int		FileSystemLocal::listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list) {
	cweeStr exten = getExtension(extension);
	exten = exten.Right(exten.Length() - 1);

	std::vector<std::string> in = DirectoryFiles(directory).Files();
	list.Clear();
	for (auto& x : in) {
		cweeStr temp = cweeStr(x.c_str());
		cweeStr extenI; temp.ExtractFileExtension(extenI);
		if (extenI.Icmp(exten) == false || exten == "*")	list.Append(cweeStr(x.c_str()));
	}
	return in.size();
}
cweeList<cweeStr> FileSystemLocal::listFilesWithExtension(const char* directory, const char* extension) {
	cweeList<cweeStr> list;
	cweeStr exten = cweeStr(extension);
	exten = exten.Right(exten.Length() - 1);
	std::vector<std::string> in = DirectoryFiles(directory).Files();
	for (auto& x : in) {
		cweeStr temp = cweeStr(x.c_str());
		cweeStr extenI; temp.ExtractFileExtension(extenI);
		if (extenI.Icmp(exten) == false || exten == "*" || exten.IsEmpty() == true)	list.push_back(x.c_str());
	}
	return list;
}
cweeList<cweeStr> FileSystemLocal::listFilesWithExtension(const char* directory, fileType_t extension) {
	cweeList<cweeStr> list;
	cweeStr exten = getExtension(extension);
	exten = exten.Right(exten.Length() - 1);
	std::vector<std::string> in = DirectoryFiles(directory).Files();
	for (auto& x : in) {
		cweeStr temp = cweeStr(x.c_str());
		cweeStr extenI; temp.ExtractFileExtension(extenI);
		if (extenI.Icmp(exten) == false || exten == "*" || exten.IsEmpty() == true)	list.push_back(x.c_str());
	}
	return list;
}

/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
void		FileSystemLocal::saveWindowsPassword(cweeStr const& account, cweeStr const& username, cweeStr  const& password) {
	cweeStr Init = account;		char* acc = (char*)Init.c_str();
	cweeStr Init2 = username;	char* user = (char*)Init2.c_str();
	cweeStr Init3 = password;	char* pass = (char*)Init3.c_str();
#ifdef CredWrite
	DWORD cbCreds = 1 + strlen(pass);
	CREDENTIAL cred = { 0 };
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = acc;
	cred.CredentialBlobSize = cbCreds;
	cred.CredentialBlob = (LPBYTE)pass;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.UserName = user;
	BOOL ok = ::CredWrite((&cred), 0);
#endif
};
/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
cweeStr		FileSystemLocal::retrieveWindowsPassword(cweeStr const& account, cweeStr const& username) {
	cweeStr Init = account;
	char* acc = (char*)Init.c_str();
	cweeStr Init2 = username;
	char* user = (char*)Init2.c_str();
#ifdef CredRead
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
#endif
	return "";
};
cweeStr		FileSystemLocal::appendLog(cweeStr const& filePath, cweeStr const& newLine) {
	ensureDirectoryExists(filePath);
	cweeStr log;
	readFileAsCweeStr(log, filePath);
	log += "\n";
	log += newLine;
	writeFileFromCweeStr(filePath, log);
	return log;
};
void		FileSystemLocal::copyFile(cweeStr const& filePathOrig, cweeStr const& filePathNew) {
	ensureDirectoryExists(filePathNew);
	AUTO g = GuardFile(filePathOrig);
	AUTO g2 = GuardFile(filePathNew);
	CopyFileA(filePathOrig.c_str(), filePathNew.c_str(), FALSE);
};
bool		FileSystemLocal::checkFileExists(cweeStr const& filePath) {
	bool exists = false;

	AUTO g = GuardFile(filePath);

	std::ifstream f;
	f.open(filePath);
	if (f.good()) {
		exists = true;
	}
	f.close();

	return exists;
};

void		FileSystemLocal::submitToast(cweeStr const& title, cweeStr const& content) {
	cweeToasts->submitToast(title, content);
};
cweeThreadedList< std::pair<cweeStr, cweeStr>> FileSystemLocal::getToasts() {
	cweeThreadedList< std::pair<cweeStr, cweeStr>> out;

	cweeStr title; cweeStr content;
	while (cweeToasts->tryGetToast(title, content)) {
		out.Append(std::make_pair(title, content));
	}
	return out;
};

std::map<std::string, cweeThreadedList<cweeStr>> FileSystemLocal::readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders) {
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
std::map<int, cweeThreadedList<std::pair<u64, float>>> FileSystemLocal::readSCADA(const cweeStr& filePath, bool myDataHasHeaders) {
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
				fileParser[i - 1].Clear();
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

cweeThreadedList<std::pair<cweeStr, cweeStr>> FileSystem::ParseJson(cweeStr source, bool* failed) {
	cweeThreadedList<std::pair<cweeStr, cweeStr>> out;

	{
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
				command += R"code(
					try{
						var& asJson = from_json(input);
						var toReturn = string("");
						for (PR : asJson){
							if (toReturn.size() > 0){ toReturn += "::externalDelim::"; }
							try{									
								toReturn += "${PR.first.to_string()}::internalDelim::${PR.second.to_string()}";
							}catch(e){}
						}
						return toReturn;
					}
					return string();
				)code";
			}
			command += "\n}";
			command.ReduceSpaces(true);
		}

		chaiscript::ChaiScript scriptEngine;

		chaiscript::Boxed_Value reply;
		if (failed) { *failed = true; }
		try {
			reply = scriptEngine.eval(command.c_str());
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

		std::string* reply2 = scriptEngine.boxed_cast<std::string*>(reply);
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
};;


cweeSharedPtr<FileSystem> fileSystem = make_cwee_shared<FileSystemLocal>(new FileSystemLocal()).CastReference<FileSystem>();

