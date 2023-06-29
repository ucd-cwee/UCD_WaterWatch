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

#include "WaterWatch_Win32_Console.h"
#include "../ExcelInterop/Wrapper.h"

class Win32ConsoleSupport {
public:
	static void clearLine() {
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;

		GetConsoleScreenBufferInfo(console, &screen);

		COORD pos = { 0, screen.dwCursorPosition.Y };

		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * 1, pos, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * 1, pos, &written
		);
		SetConsoleCursorPosition(console, pos);
	};
};

/* Linux style terminal for multithread friendly processing. */
static cweeJob GetUserInput() {
	cweeJob out([]() {
		std::cout << std::endl; // new line, since we'll be refreshing constantly on the current line, then using carriage return to reset the origin.
		cweeStr temp;
		while (1) {
			// if the user hasn't typed anything, keep thinking.                
			Win32ConsoleSupport::clearLine();
			Win32ConsoleSupport::clearLine();
			std::cout << "\r" << temp; // print the current text here, so that it doesn't appear to flash on screen.
			while (!_kbhit()) {}
			unsigned char character = _getch(); // grab user key press. 	
			// bool shift_pressed = GetAsyncKeyState(VK_SHIFT);

			if (character == '\n' || character == '\r') { // new line
				std::cout << std::endl;
				break;
			}
			if (character == '\b') { // backspace
				temp = temp.Left(cweeMath::max(0, temp.Length() - 1)); // remove one character
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if ((int)character == 27) { // escape
				temp.Clear();
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if (character == 0 || character == 0xE0) {
				character = _getch();

				if (character == 72) { // up arrow		
					continue;
				}
				if (character == 80) { // down arrow	
					continue;
				}
				if (character == 75) { // left arrow	
					continue;
				}
				if (character == 77) { // right arrow	
					continue;
				}
				continue; // If another not-programmed key was pressed, skip. (i.e. Home button, PGUP, END, PGDOWN)
			}

			temp += (char)character; // all other keys, record to temp repo
		}
		return temp;
		});
	return out.AsyncInvoke();
};
static cweeStr UserMustSelectFile(fileType_t fileType = fileType_t::ANY_EXT) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return fileSystem->getDataFolder() + "\\" + files[(int)reply];
	}
	else {
		return fileSystem->getDataFolder() + "\\" + reply.BestMatch(files);
	}
};
static cweeStr UserMustSelectFile(cweeStr fileType) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return files[(int)reply];
	}
	else {
		return reply.BestMatch(files);
	}
};

/* Parallel thread to occasionally look for and process toast messages. Sleeps most of the time and wakes up to check for toasts. */
static DispatchTimer parallel_toast_manager = DispatchTimer(100, cweeJob([](cweeStr& title, cweeStr& content) { 
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}, cweeStr(), cweeStr()));

// Handle async or scripted AppRequests. 
static DispatchTimer AppLayerRequestProcessor = DispatchTimer(100, cweeJob([]() {
	std::pair<
		int, // ID
		cweeUnion<
			cweeStr, // Function Name
			cweeThreadedList<cweeStr>, // Arguments (optional)
			cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
#pragma warning(disable : 4573)			// the usage of 'cweeStr::Hash' requires the compiler to capture 'this' but the current default capture mode does not allow it
		cweeStr result;
		cweeStr& funcName = request.second.get<0>();
		cweeThreadedList<cweeStr>& args = request.second.get<1>();
		switch (static_cast<size_t>(funcName.Hash())) {
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFile")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFolder")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SavePassword")): // server, username, password
			if (args.Num() >= 3) fileSystem->saveWindowsPassword(args[0], args[1], args[2]);
			else result = "Arguments required: 'account_name', 'user_name', 'password'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_LoadPassword")):
			if (args.Num() >= 2) result = fileSystem->retrieveWindowsPassword(args[0], args[1]);
			else result = "Arguments required: 'account_name', 'user_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_ThemeColor")):
			if (args.Num() >= 1) result = "[255,255,255,255]";
			else result = "Arguments required: 'color_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetUserName")):
			result = "Win32 Project";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetMousePosition")):
			result = "[0,0]";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SaveSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetSetting")):
			result = "TBD";
			break;
		default:
			// unknown function.
			result = "Function Not Found";
			break;
		}
		AppLayerRequests->FinishRequest(request.first, result); // let the app set the result and end this dequeue
#pragma warning(default : 4573)	
	}
}));

static cweeStr GetHeaderString() {
	cweeStr toRet = "Welcome to the WaterWatch Sample App.\n";
	toRet += "Data Directory = " + fileSystem->getDataFolder() + "\n";
	toRet += "Begin Scripting:\n\n";
	return toRet;
}


int main() {
	using namespace cwee_units;


#if 1
	fileSystem->ensureDirectoryExists(fileSystem->getDataFolder());
	cweeStr filePath = fileSystem->getDataFolder() + "\\excelTest\\excelExample.xlsx";

	if (!fileSystem->checkFileExists(filePath)) {
		std::cout << "The Excel File Could Not Be Found At: " << filePath << std::endl;
	}
	else {		
		cweeSharedPtr<ExcelWorkbook> workbook;
		if (true) {
			workbook = cweeExcel::OpenExcel();
			workbook->load(filePath);
		}
		else {
			workbook = cweeExcel::OpenExcel(filePath); // identical
		}

		// Goal 1: Fill a simple matrix of strings from the active worksheet from the excel document
		{
			cweeList<cweeList<cweeStr>> matrix;
			{
				AUTO worksheet = workbook->active_sheet();
				AUTO range = worksheet->rows(); // used to iterate by rows 
				for (auto& row : *range) {
					cweeList<cweeStr>& rowOfStrings = matrix.Alloc();
					for (auto& cell : row) {
						rowOfStrings.Append(cell->to_string());
					}
				}
			}
			// print the first 10 rows and 10 columns to the console as an example
			for (int i = 0; i < 10 && i < matrix.Num(); i++) {
				cweeStr rowS;
				for (int j = 0; j < 10 && j < matrix[i].Num(); j++) {
					rowS.AddToDelimiter(matrix[i][j], ", ");
				}
				std::cout << rowS << std::endl;
			}
			std::cout << std::endl;
		}

		// Goal 2: Parse the Excel document (KNOWING ITS FORMAT) into a series of time-series patterns and report on their statistics
		{
			using TimeSeriesType = cweeBalancedPattern<units::dimensionless::scalar_t>; // Time to Measurement .. setting measurement units to "dimensionless" for now.
			using SiteMeasurementType = cweeThreadedMap<cweeStr, TimeSeriesType>; // AL to TimeSeries
			using SiteCollectionType = cweeThreadedMap<cweeStr, SiteMeasurementType>; // AA-01 to SiteMeasurementType
			
			SiteCollectionType SiteCollection;
			for (auto& worksheet : *workbook) {				
				int rowNum = 0;
				cweeStr siteName = worksheet.title();
				AUTO siteMeasurement = SiteCollection.GetPtr(siteName); // gets a shared PTR
				if (siteMeasurement) {
					cweeList<cweeStr> header;
					try {
						AUTO rows = worksheet.rows();
						for (auto& row : *rows) {
							rowNum++;
							if (rowNum <= 1) {
								for (auto& cell : row) {
									header.Append(cell->to_string());
								}
							}
							else {
								int colNum = 0; cweeTime time;
								for (auto& cell : row) {
									colNum++;

									switch (colNum) {
									case 1: break;
									case 2: {
										try {
											time = cell->value<cweeTime>();
										}
										catch (...) {}
										break;
									}
									default: {
										cweeStr& headerForThisCell = header[colNum - 1];

										AUTO TM = siteMeasurement->GetPtr(headerForThisCell);
										if (TM) {
											try {
												TM->AddValue((u64)time, cell->value<double>());
											}
											catch (...) {}
										}
										break;
									}
									}
								}
							}
						}
					}catch(...){}
				}
			}

			// report statistics
			cweeList<float> quantiles; quantiles = { 0.05f, 0.50f, 0.95f };
			for (auto& site : SiteCollection) {
				auto& siteName = site.first;
				auto& siteMeasurements = site.second;
				if (siteMeasurements) {
					for (auto& measurement : *siteMeasurements) {
						auto& measurementName = measurement.first;
						auto& measurementPattern = measurement.second;
						if (measurementPattern) {
							AUTO value_quantiles = measurementPattern->ValueQuantiles(quantiles);
							for (int i = 0; i < quantiles.Num(); i++) {
								std::cout << cweeStr::printf("%s: %s: %f percentile: %f", siteName.c_str(), measurementName.c_str(), quantiles[i], (float)value_quantiles[i]()) << std::endl;
							}
						}
					}
				}
			}











		}


	}


#endif









	std::cout << GetHeaderString() << std::endl;

	cweeStr prevLine = "";
	cweeStr command = "";
	cweeSharedPtr<chaiscript::WaterWatch_ChaiScript> engine = make_cwee_shared<chaiscript::WaterWatch_ChaiScript>();
	while (true) {
		AUTO input = GetUserInput();
		cweeStr str = input.Await().cast();

		if (str == "Exit")
		{
			return 0;
		}
		if (str == "Reset") { 
			engine = make_cwee_shared<chaiscript::WaterWatch_ChaiScript>();
			command = ""; 
			continue; 
		} 
		if (str == "" && prevLine == "") {
			Stopwatch sw;
			sw.Start();

			cweeStr result;
			try {
				AUTO bv = engine->eval(command.c_str());
				result = engine->to_string(bv).c_str();
			} catch (std::exception e) {
				result = e.what();
			}

			sw.Stop();
			std::cout << "Time Elapsed; " << (second_t)sw.Seconds_Passed() << std::endl;
			std::cout << "Current DateTime: " << ((cweeTime)fileSystem->getCurrentTime()).c_str() << std::endl;

			std::cout << result << std::endl;

			command = "";
		}

		command += str;
		command += "\n";
		prevLine = str;
	}
}
