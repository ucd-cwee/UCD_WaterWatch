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
#include "../FiberTasks/FiberTasks.h"

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
		//case static_cast<size_t>(cweeStr::Hash("BoostFiber")):
			//result = std::to_string(FTL::fnBoostFiber()).c_str();
			//break;
		case static_cast<size_t>(cweeStr::Hash("Fiber")): 
			result = std::to_string(FTL::fnFiberTasks()).c_str();
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber2a")):
			if (args.Num() >= 1) result = std::to_string(FTL::fnFiberTasks2a(args[0].ReturnNumeric())).c_str();
			else result = "Arguments required: 'num_tasks'";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber2b")):
			if (args.Num() >= 1) result = std::to_string(FTL::fnFiberTasks2b(args[0].ReturnNumeric())).c_str();
			else result = "Arguments required: 'num_tasks'";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber2c")):
			if (args.Num() >= 1) result = std::to_string(FTL::fnFiberTasks2c(args[0].ReturnNumeric())).c_str();
			else result = "Arguments required: 'num_tasks'";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber2d")):
			if (args.Num() >= 1) result = std::to_string(FTL::fnFiberTasks2d(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
			else result = "Arguments required: 'num_tasks', 'num_subtasks'";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber3")):
			if (args.Num() >= 2) result = std::to_string(FTL::fnFiberTasks3(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
			else result = "Arguments required: 'num_tasks', 'num_subtasks'";
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

#define AddFunctionToLib(lib, name, todo, ...){ \
	auto varNames = cweeStr(#__VA_ARGS__).RemoveBetween("<", ">").ReplaceInline("*", " ").ReplaceInline("&", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").Split(",").Trim(' ').ReplaceInline("  ", " ").SplitAgain(" ").Trim(' ').GetEveryOtherVar(); \
	lib->add(fun([](__VA_ARGS__) { todo; }, varNames), #name);	\
}

int main() {
	using namespace cwee_units;
	
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
