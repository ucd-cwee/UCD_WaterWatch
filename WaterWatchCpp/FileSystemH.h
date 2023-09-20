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
#include "Precompiled.h"
#include "enum.h"
#include "Strings.h"
#include "vec.h"
#include "SharedPtr.h"
#include "List.h"
#include "cweeTime.h"

BETTER_ENUM(fileType_t, uint8_t, ANY_EXT, INP, TXT, CSV, EXL, DAT, ZIP, ZIP_7z, sqlDB, EDMS, PK4, PROJECT, PATFILE);

class IpAddressInformation {
public:
	cweeStr city;
	cweeStr country_name;
	cweeStr ip;
	cweeStr latitude;
	cweeStr longitude;
	cweeStr region_name;
	cweeStr time_zone;
	cweeStr zip_code;
	vec3d	coordinates; // longitude, latitude, elevation
};

class FileSystem {
private:
	virtual void ClearTempFiles() = 0;

public: // File Locks
	virtual void LockFile(cweeStr const& filePath) = 0;
	virtual void UnlockFile(cweeStr const& filePath) = 0;
	virtual cweeSharedPtr<cweeStr> GuardFile(cweeStr const& filePath) = 0;

protected:
	virtual cweeStr	getUniqueFingerprint() = 0;
	static cweeStr	getApplicationPathAndName() {
		wchar_t wtext[2048];
		LPWSTR buffer(wtext);
		::GetModuleFileNameW(0, buffer, 2048);
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
	static bool				ensureDirectoryExists(const cweeStr& Directory) {
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
	};;
	virtual void			removeFile(const cweeStr& filePath) = 0;
	virtual void			renameFile(const cweeStr& oldFilePath, const cweeStr& newFilePath) = 0;
	virtual cweeStr			getAppFolder() = 0;
	virtual cweeStr			getAppName() = 0;
	virtual cweeStr			getDataFolder() = 0;
	virtual cweeStr			setDataFolder(cweeStr const& in) = 0;
	virtual cweeStr			createFilePath(cweeStr const& directory, cweeStr const& fileName, fileType_t const& fileType) = 0;
	virtual cweeStr			createRandomFilePath(fileType_t const& fileType) = 0;
	virtual cweeStr			createRandomFile(fileType_t const& fileType) = 0;
	virtual cweeStr			createRandomFile(cweeStr const& fileType) = 0;
	virtual u64				getCurrentTime() = 0;
	static cweeTime			localtime(const u64& time) {
		return cweeTime(time);
	};;
	static cweeTime			gmtime(const u64& time) {
		return cweeTime(time);
	};;

	virtual cweeStr			QueryHttp(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "WaterWatchCpp") = 0;
	virtual cweeStr			QueryHttpToFile(const cweeStr& mainAddress = "nationalmap.gov", const cweeStr& requestParameters = "epqs/pqs.php?y=-117&x=33&output=xml&units=Feet", const cweeStr& UniqueSessionName = "WaterWatchCpp") = 0;
	virtual cweeStr			GetIpAddress() = 0;
	virtual IpAddressInformation			GetAddress() = 0;
	static cweeThreadedList<std::pair<cweeStr, cweeStr>> ParseJson(cweeStr source, bool* failed = nullptr);

public:
	virtual cweeThreadedList<cweeStr>	readFileAsStrList(cweeStr const& filePath) = 0;
	virtual void	readFileAsCweeStr(cweeStr& out, cweeStr const& filePath) = 0;
	virtual void	writeFileFromStrList(cweeStr const& filePath, cweeThreadedList<cweeStr> const& content) = 0;
	virtual void	writeFileFromCweeStr(cweeStr const& filePath, cweeStr const& content) = 0;
	virtual void	writeFileFromCweeStr(cweeStr const& folderPath, cweeStr const& fileName, fileType_t const& fileType, cweeStr const& content) = 0;

	static size_t	fileSys_write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
		size_t written = std::fwrite(ptr, size, nmemb, stream);
		return written;
	};

	virtual void	downloadFileFromURL(cweeStr const& url, cweeStr const& destinationFilePath) = 0;
	virtual cweeStr DownloadCweeStrFromURL(cweeStr const& url) = 0;

public:
	virtual int		getByteSizeOfFile(cweeStr const& filePath) = 0;
	virtual cweeStr ReadFirstFileFromZipToString(cweeStr const& zipFilePath) = 0;
	virtual cweeStr ReadFileFromZipToString(cweeStr const& zipFilePath, int whichStartingZero) = 0;
	virtual cweeStr	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName) = 0;
	virtual void	ReadFileFromZipToString(cweeStr const& zipFilePath, cweeStr const& fileName, cweeStr & out) = 0;
	virtual void	addContentToZip(cweeStr const& zipFilePath, const cweeStr& fileName, cweeStr& content) = 0;
	virtual void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeStr const& exceptFileName) = 0;
	virtual void	copyContentBetweenZips(cweeStr const& DestinationZipFilePath, cweeStr const& SourceZipFilePath, cweeThreadedList<cweeStr> const& exceptFileNames) = 0;
	virtual void	writeStrListToZipFile(cweeStr const& zipFilePath, const cweeThreadedList<cweeStr>& data) = 0;
	virtual cweeList<cweeStr> getFileNamesInZip(cweeStr const& filePath) = 0;
	virtual int		getNumFilesInZip(cweeStr const& zipFilePath) = 0;
	virtual void	addFileToZip(cweeStr const& zipFilePath, cweeStr const& fileToAddPath) = 0;

public:
	virtual int		listOSFiles(const char* directory, fileType_t extension, cweeThreadedList<cweeStr>& list) = 0;
	virtual cweeList<cweeStr> listFilesWithExtension(const char* directory, const char* extension) = 0;
	virtual cweeList<cweeStr> listFilesWithExtension(const char* directory, fileType_t extension) = 0;

public:
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	virtual void		saveWindowsPassword(cweeStr const& account, cweeStr const& username, cweeStr  const& password) = 0;
	/* Does not work with sandbox environments like UWP. Only works in Win32 environment. */
	virtual cweeStr		retrieveWindowsPassword(cweeStr const& account, cweeStr const& username) = 0;
	virtual cweeStr		appendLog(cweeStr const& filePath, cweeStr const& newLine) = 0;
	virtual void		copyFile(cweeStr const& filePathOrig, cweeStr const& filePathNew) = 0;
	virtual bool		checkFileExists(cweeStr const& filePath) = 0;

	virtual void		submitToast(cweeStr const& title, cweeStr const& content) = 0;
	virtual cweeThreadedList< std::pair<cweeStr, cweeStr>> getToasts() = 0;

public:
	virtual std::map<std::string, cweeThreadedList<cweeStr>> readCommaSeperatedValues(const cweeStr& filePath, bool myDataHasHeaders = true) = 0;
	virtual std::map<int, cweeThreadedList<std::pair<u64, float>>> readSCADA(const cweeStr& filePath, bool myDataHasHeaders = true) = 0;

};
extern cweeSharedPtr<FileSystem> fileSystem;