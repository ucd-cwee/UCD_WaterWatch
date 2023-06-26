#pragma once
#include "Precompiled.h"

BETTER_ENUM(fileType_t, uint8_t, ANY_EXT, INP, TXT, CSV, EXL, DAT, ZIP, ZIP_7z, sqlDB, EDMS, PK4, PROJECT, PATFILE);

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


class FileSystem {
private:
	static constexpr long								MAX_ZIPPED_FILE_NAME = 2048;
	static constexpr long								FILE_HASH_SIZE = 1024;
	static constexpr chaiscript::utility::Static_String privateServerKey = chaiscript::utility::Static_String("yevUcBl^KyeCO*WUqF6}n:Ci/RzcmqLiT^#>IZ!f"); // public should be == jow0MY5igjre>]VY*P=5COS0$1Ztk]3<R<1MgrPT
	static constexpr long								currentRealTimeAccuracyMilliseconds = 100;
	const cweeStr										ip_address;
	// const IpAddressInformation							ipAddressInfo;
	const u64											timeZoneAdjust;
	const cweeTime										epoch; // 1970/1/1 00:00:00
	const cweeStr 										appFolder;
	const cweeStr										appName;
	cweeUnpooledInterlocked < cweeStr >					dataFolder;
	cweeSharedPtr < cweeTime >							currentRealTime; // updated automatically by currentRealTimeUpdater.
	DispatchTimer										currentRealTimeUpdater; // updates within the accuracy of 'currentRealTimeAccuracyMilliseconds' ... thread will automatically die with this class instance.
	cweeThreadedMap<cweeStr, cweeSysMutex>				fileLocks;
	cweeInterlocked<cweeList<cweeStr>>					tempFiles;
	//zmq::context_t zeroMQ_context = zmq::context_t(1);
	//cweeUnorderedList< cweeSocket > publishingSockets;
	//cweeUnorderedList< cweeSocket > subscribingSockets;
	//cweeUnorderedList< cweeRouter > routerSockets;
	//cweeUnorderedList< cweeSocket > clientSockets;
	//
	//cweeUnorderedList< cweeRouterRequest > routerRequests;

public:
	FileSystem() :
		timeZoneAdjust((boost::posix_time::microsec_clock::local_time() - boost::posix_time::microsec_clock::universal_time()).total_milliseconds() / 1000.0)
		, epoch(boost::posix_time::time_from_string("1970/1/1 0:0:0"))
		, appFolder(getApplicationPath())
		, appName(getApplicationName())
		, dataFolder(setDataFolder(appFolder + cweeStr("data\\")))
		, currentRealTime(new cweeTime())
		, currentRealTimeUpdater(currentRealTimeAccuracyMilliseconds, cweeJob([](cweeSharedPtr<cweeTime> ptr) { u64 t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0; AUTO g = ptr.Guard(); *ptr.UnsafeGet() = t; }, currentRealTime))
		, fileLocks()
		, ip_address(QueryHttp("api.ipify.org", ""))
	    // , ipAddressInfo(GetIpAddressInformationActual())
	{};
	~FileSystem() {
		ClearTempFiles();
	};
private:
	void ClearTempFiles() {
		AUTO e = this->tempFiles.GetExclusive();
		for (auto& x : *e) {
			removeFile(x);
		}
		e->Resize(0);
	};

public: // File Locks
	void LockFile(cweeStr const& filePath) {
		fileLocks[filePath]->Lock(); // if the directory does not already exist, it will shortly.
	};
	void UnlockFile(cweeStr const& filePath) {
		fileLocks[filePath]->Unlock();
	};
	NODISCARD AUTO GuardFile(cweeStr const& filePath) {
		LockFile(filePath);
		return cweeSharedPtr<cweeStr>(new cweeStr(filePath), [this](cweeStr* p) { this->UnlockFile(*p); delete p; });
	};

private:
	cweeStr			getUniqueFingerprint() {
		cweeStr MAC_ADDRESS;
		cweeStr IP_ADDRESS;
		{
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
		}

		//cweeStr randomID = cweeStr::printf("%04X-%04X", cweeRandomInt(0x10000), cweeRandomInt(0x10000)); // ideally we don't want this... 

		return MAC_ADDRESS + "-" + IP_ADDRESS;// +"-" + randomID;
	};
	static cweeStr	getApplicationPathAndName() {
		wchar_t wtext[MAX_ZIPPED_FILE_NAME];
		LPWSTR buffer(wtext);
		::GetModuleFileNameW(0, buffer, MAX_ZIPPED_FILE_NAME);
		std::wstring tempStr(buffer);
		std::string FileDir(std::string(tempStr.begin(), tempStr.end()).erase(std::string(tempStr.begin(), tempStr.end()).length()));
		return cweeStr(FileDir.c_str());
	};
	static cweeStr	getApplicationPath() {
		cweeStr appPath;
		getApplicationPathAndName().ExtractFilePath(appPath);
		return appPath;
	};
	static cweeStr	getApplicationName() {
		cweeStr appName;
		getApplicationPathAndName().ExtractFileBase(appName);
		return appName;
	};
	static cweeStr	getExtension(fileType_t const& fileType) {
		switch (fileType) {
			case fileType_t::INP:		return ".inp"; 
			case fileType_t::TXT:		return ".txt";
			case fileType_t::CSV:		return ".csv"; 
			case fileType_t::EXL:		return ".xlxs";
			case fileType_t::DAT:		return ".dat";
			case fileType_t::ZIP:		return ".zip";
			case fileType_t::ZIP_7z:	return ".7z";
			case fileType_t::sqlDB:		return ".db";
			case fileType_t::EDMS:		return ".edmsTxt";
			case fileType_t::PK4:		return ".pk4";
			case fileType_t::PROJECT:	return ".water";
			case fileType_t::PATFILE:	return ".pattern"; 
			default:					return ".*"; 
		}
	};

public:
	static bool		ensureDirectoryExists(const cweeStr& Directory) {
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
	void			removeFile(const cweeStr& filePath) {
		LockFile(filePath);
		std::remove(filePath);
		UnlockFile(filePath);
	};
	void			renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath) {
		LockFile(oldFilePath);
		std::rename(oldFilePath, newFilePath);
		UnlockFile(oldFilePath);
	};
	cweeStr			getAppFolder() { return appFolder; };
	cweeStr			getAppName() { return appName; };
	cweeStr			getDataFolder() { return dataFolder; };
	cweeStr			setDataFolder(cweeStr const& in) {
		ensureDirectoryExists(in);
		dataFolder = in;
		return in;
	};
	cweeStr			createFilePath(cweeStr const& directory, cweeStr const& fileName, fileType_t const& fileType) {
		ensureDirectoryExists(directory);
		return directory + "\\" + fileName + getExtension(fileType);
	};
	cweeStr			createRandomFilePath(fileType_t const& fileType) {
		return createFilePath(getDataFolder() + "\\temp", cweeStr::printf("%i_%i_%i", cweeRandomInt(0,1000), cweeRandomInt(0, 1000), cweeRandomInt(0, 1000)), fileType);
	};
	cweeStr			createRandomFile(fileType_t const& fileType) {
		cweeStr filePath = createFilePath(getDataFolder() + "\\temp", cweeStr::printf("%i_%i_%i", cweeRandomInt(0, 1000), cweeRandomInt(0, 1000), cweeRandomInt(0, 1000)), fileType);
		this->writeFileFromCweeStr(filePath, "");
		this->tempFiles.GetExclusive()->Append(filePath);
		return filePath;
	};
	u64				getCurrentTime() {
		u64 out(0);
		AUTO g = currentRealTime.Guard();
		out = currentRealTime.UnsafeGet()->operator u64();
		return out;
	};
	static cweeTime	localtime(const u64& time) {
		return cweeTime(time);
	};
	static cweeTime gmtime(const u64& time) {
		return cweeTime(time);
	};

	static cweeStr  QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "WaterWatchCpp") {
		return geocoding->QueryHttp(mainAddress, requestParameters, UniqueSessionName);
	};
	cweeStr			GetIpAddress() {
		return ip_address;		
	};
	static cweeThreadedList<std::pair<cweeStr, cweeStr>> ParseJson(cweeStr source, bool* failed = nullptr) {
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
	};

public:
	cweeThreadedList<cweeStr>	readFileAsStrList(cweeStr const& filePath) {
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
		UnlockFile(filePath);
		return collection;
	};
	void	readFileAsCweeStr(cweeStr& out, cweeStr const& filePath) {
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
	void	writeFileFromStrList(cweeStr const& filePath, cweeThreadedList<cweeStr> const& content) {
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
	void	writeFileFromCweeStr(cweeStr const& filePath, cweeStr const& content) {
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
	void	writeFileFromCweeStr(cweeStr const& folderPath, cweeStr const& fileName, fileType_t const& fileType, cweeStr const& content) {
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

	static size_t fileSys_write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
		size_t written = std::fwrite(ptr, size, nmemb, stream);
		return written;
	};	

	void	downloadFileFromURL(cweeStr const& url, cweeStr const& destinationFilePath) {
		ensureDirectoryExists(destinationFilePath);
#if 1	
		writeFileFromCweeStr(destinationFilePath, QueryHttp(url, ""));
#else
		{
			CURL* curl;
			FILE* fp;
			CURLcode result;
			char* Url = (char*)url.c_str();

			curl = curl_easy_init();
			if (curl) {
				LockFile(destinationFilePath);
				{ // do something with the file
					fp = fopen(destinationFilePath, "wb");
					curl_easy_setopt(curl, CURLOPT_URL, Url);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileSys_write_data);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
					curl_easy_perform(curl);
					/* always cleanup */
					curl_easy_cleanup(curl);
					fclose(fp);
				}
				UnlockFile(destinationFilePath);
			}
		}
#endif
	};
	cweeStr DownloadCweeStrFromURL(cweeStr const& url) {
		cweeStr content;
		{
			AUTO randPath = createRandomFilePath(fileType_t::TXT);
			downloadFileFromURL(url, randPath);
			readFileAsCweeStr(content, randPath);
			removeFile(randPath);
		}
		return content;
	};

public:
	int		getByteSizeOfFile(cweeStr const& filePath) {
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
	cweeStr ReadFirstFileFromZipToString(cweeStr const& zipFilePath) {

		unzipper zipFile;

		AUTO g = GuardFile(zipFilePath);

		zipFile.open(zipFilePath);
		cweeStr digitized;
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

		return digitized;
	};
	cweeStr ReadFileFromZipToString(cweeStr const& zipFilePath, int whichStartingZero) {
#define unzip_inMem
		unzipper zipFile;


		AUTO g = GuardFile(zipFilePath);

		zipFile.open(zipFilePath);
		cweeStr digitized;
		int numFiles = 0;

		auto list = zipFile.getFilenames();

		std::sort(list.begin(), list.end(), [](std::string const& a, std::string const& b)->bool { return a < b; }); //sort the vector alphebetically

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

		return digitized;
#undef unzip_inMem
	};
	cweeStr	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName) {
		cweeStr out;
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

		return out;
	};;;
	void	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName, cweeStr& out) {
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

	};
	void	addContentToZip(cweeStr const& zipFilePath, const cweeStr& fileName, cweeStr& content) {
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
	};;
	void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeStr const& exceptFileName) {
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
	};
	void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeThreadedList<cweeStr> const& exceptFileNames) {
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
	};;
	void	writeStrListToZipFile(cweeStr const& zipFilePath, const cweeThreadedList<cweeStr>& data) {
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
	};
	cweeList<cweeStr> getFileNamesInZip(cweeStr const& filePath) {
		cweeThreadedList<cweeStr> out;

		unzipper UnzipFile; int ijk;

		AUTO g = GuardFile(filePath);

		if (UnzipFile.open(filePath))
		{
			for (auto& x : UnzipFile.getFilenames()) {
				out.Append(x.c_str());
			}
		}
		UnzipFile.close();

		return out;
	};
	int		getNumFilesInZip(cweeStr const& zipFilePath) {
		int toReturn = -1;

		AUTO g = GuardFile(zipFilePath);



		unzFile zip = unzOpen(zipFilePath);
		if (zip) toReturn = unzGetNumFiles(zip);
		unzClose(zip);

		return toReturn;
	};
	void	addFileToZip(cweeStr const& zipFilePath, cweeStr const& fileToAddPath) {
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
	};

public:
	int		listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list) {
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
	cweeList<cweeStr> listFilesWithExtension(const char* directory, const char* extension) {
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
	cweeList<cweeStr> listFilesWithExtension(const char* directory, fileType_t extension) {
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
public:
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	void		saveWindowsPassword(cweeStr const& account, cweeStr const& username, cweeStr  const& password) {
		cweeStr Init = account;		char* acc = (char*)Init.c_str();
		cweeStr Init2 = username;	char* user = (char*)Init2.c_str();
		cweeStr Init3 = password;	char* pass = (char*)Init3.c_str();

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
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	cweeStr		retrieveWindowsPassword(cweeStr const& account, cweeStr const& username) {
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
	cweeStr		appendLog(cweeStr const& filePath, cweeStr const& newLine) {
		ensureDirectoryExists(filePath);
		cweeStr log;
		readFileAsCweeStr(log, filePath);
		log += "\n";
		log += newLine;
		writeFileFromCweeStr(filePath, log);
		return log;
	};
	void		copyFile(cweeStr const& filePathOrig, cweeStr const& filePathNew) {
		ensureDirectoryExists(filePathNew);		
		AUTO g = GuardFile(filePathOrig);
		AUTO g2 = GuardFile(filePathNew);
		CopyFile(filePathOrig.c_str(), filePathNew.c_str(), FALSE);
	};
	bool		checkFileExists(cweeStr const& filePath) {
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

	void		submitToast(cweeStr const& title, cweeStr const& content) {
		cweeToasts::submitToast(title, content);
	};
	cweeThreadedList< std::pair<cweeStr, cweeStr>> getToasts() {
		cweeThreadedList< std::pair<cweeStr, cweeStr>> out;

		cweeStr title; cweeStr content;
		while (cweeToasts::tryGetToast(title, content)) {
			out.Append( std::make_pair( title, content ) );
		}		
		return out;
	};

public:
	std::map<std::string, cweeThreadedList<cweeStr>> readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders = true) {
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
	std::map<int, cweeThreadedList<std::pair<u64, float>>> readSCADA(const cweeStr& filePath, bool myDataHasHeaders = true) {
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

#if 0
private:
	/* implimentation - required functions : */
private:
	IpAddressInformation GetIpAddressInformationActual() {
		IpAddressInformation out;
		cweeThreadedList<std::pair<cweeStr, cweeStr>> data;
		cweeStr read;
		bool failed = true;

#pragma region "Internet Data"
		int maxTries = 20;
		while (failed && (--maxTries > 0)) {
			cweeStr IPaddress = ip_address;
			cweeStr destinationFilePath = createRandomFilePath(fileType_t::TXT);
			{
				CURL* curl;
				FILE* fp;
				CURLcode result;
				curl = curl_easy_init();
				if (curl) {
					LockFile(destinationFilePath);
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
					UnlockFile(destinationFilePath);
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
	IpAddressInformation GetIpAddressInformation() { return ipAddressInfo; };



#endif

};

static cweeSharedPtr<FileSystem> fileSystem = make_cwee_shared<FileSystem>();







