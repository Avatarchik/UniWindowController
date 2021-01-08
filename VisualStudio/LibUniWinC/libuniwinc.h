#pragma once

#ifdef LIBUNIWINC_EXPORTS
#define UNIWINC_API __stdcall
#define UNIWINC_EXPORT extern "C" __declspec(dllexport)
#else
#define UNIWINC_API __stdcall
#define UNIWINC_EXPORT extern "C" __declspec(dllimport)
#endif


// Maximum monitor number that this library could be handle
#define UNIWINC_MAX_MONITORCOUNT 32

// Maximum length for a classname
#define UNIWINC_MAX_CLASSNAME 32


// Methods to transparent the window
enum class TransparentType : int {
	None = 0,
	Alpha = 1,
	ColorKey = 2
};

// Event type (Experimental)
enum class EventType : int {
	None = 0,
	Style = 1,
	Size = 2,
	Order = 4
};

// Function called when window style (e.g. maximized, transparetize, etc.)
//   param: The argument is indicate the kind of event
using WindowStyleChangedCallback = void(*)(INT32);

// Function called when files have dropped
//   param: The argument is a \0 ended  UTF-16 string with each path separated by \n
using DropFilesCallback = void(*)(WCHAR*);

// Function called when displays have changed
//   param: The argument is the numbers of monitors
using MonitorChangedCallback = void(*)(INT32);

using AppCommandCallback = void(*)(INT32);


// Winodow state functions
UNIWINC_EXPORT BOOL UNIWINC_API IsActive();
UNIWINC_EXPORT BOOL UNIWINC_API IsTransparent();
UNIWINC_EXPORT BOOL UNIWINC_API IsBorderless();
UNIWINC_EXPORT BOOL UNIWINC_API IsTopmost();
UNIWINC_EXPORT BOOL UNIWINC_API IsBottommost();
UNIWINC_EXPORT BOOL UNIWINC_API IsBackground();
UNIWINC_EXPORT BOOL UNIWINC_API IsMaximized();
UNIWINC_EXPORT BOOL UNIWINC_API IsMinimized();

UNIWINC_EXPORT BOOL UNIWINC_API AttachMyWindow();
UNIWINC_EXPORT BOOL UNIWINC_API AttachMyOwnerWindow();
UNIWINC_EXPORT BOOL UNIWINC_API AttachMyActiveWindow();
UNIWINC_EXPORT BOOL UNIWINC_API DetachWindow();

UNIWINC_EXPORT void UNIWINC_API SetTransparent(const BOOL isTransparent);
UNIWINC_EXPORT void UNIWINC_API SetBorderless(const BOOL isBorderless);
UNIWINC_EXPORT void UNIWINC_API SetTopmost(const BOOL isTopmost);
UNIWINC_EXPORT void UNIWINC_API SetBottommost(const BOOL isBottommost);
UNIWINC_EXPORT void UNIWINC_API SetBackground(const BOOL isBackground);
UNIWINC_EXPORT void UNIWINC_API SetClickThrough(const BOOL isTransparent);
UNIWINC_EXPORT void UNIWINC_API SetMaximized(const BOOL isZoomed);
UNIWINC_EXPORT BOOL UNIWINC_API SetPosition(const float x, const float y);
UNIWINC_EXPORT BOOL UNIWINC_API GetPosition(float* x, float* y);
UNIWINC_EXPORT BOOL UNIWINC_API SetSize(const float width, const float height);
UNIWINC_EXPORT BOOL UNIWINC_API GetSize(float* width, float* height);
UNIWINC_EXPORT INT32 UNIWINC_API GetCurrentMonitor();

// Event handling
UNIWINC_EXPORT BOOL UNIWINC_API RegisterWindowStyleChangedCallback(WindowStyleChangedCallback callback);
UNIWINC_EXPORT BOOL UNIWINC_API UnregisterWindowStyleChangedCallback();
UNIWINC_EXPORT BOOL UNIWINC_API RegisterMonitorChangedCallback(MonitorChangedCallback callback);
UNIWINC_EXPORT BOOL UNIWINC_API UnregisterMonitorChangedCallback();
UNIWINC_EXPORT BOOL UNIWINC_API RegisterDropFilesCallback(DropFilesCallback callback);
UNIWINC_EXPORT BOOL UNIWINC_API UnregisterDropFilesCallback();
UNIWINC_EXPORT BOOL UNIWINC_API RegisterAppCommandCallback(AppCommandCallback callback);
UNIWINC_EXPORT BOOL UNIWINC_API UnregisterAppCommandCallback();


// Monitor Info.
UNIWINC_EXPORT INT32 UNIWINC_API GetMonitorCount();
UNIWINC_EXPORT BOOL UNIWINC_API GetMonitorRectangle(const INT32 monitorIndex, float* x, float* y, float* width, float* height);

// Mouse pointer
UNIWINC_EXPORT BOOL UNIWINC_API SetCursorPosition(const float x, const float y);
UNIWINC_EXPORT BOOL UNIWINC_API GetCursorPosition(float* x, float* y);

// File drop
UNIWINC_EXPORT BOOL UNIWINC_API SetAllowDrop(const BOOL bEnabled);

// Windows only
UNIWINC_EXPORT void UNIWINC_API SetTransparentType(const TransparentType type);
UNIWINC_EXPORT void UNIWINC_API SetKeyColor(const COLORREF color);
UNIWINC_EXPORT HWND UNIWINC_API GetWindowHandle();
UNIWINC_EXPORT HWND UNIWINC_API GetDesktopWindowHandle();
UNIWINC_EXPORT DWORD UNIWINC_API GetMyProcessId();
