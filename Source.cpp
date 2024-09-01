#include <vector>
#include "LogitechLCDLib.h"  // Library for interacting with Logitech LCD displays
#include <gdiplus.h>  // Library for working with graphics and images
#include <Windows.h>
#include <Shlwapi.h>  // Library for path handling
#include <string>
#include <thread>  // Library for creating and managing threads
#include <shellapi.h> // For tray icon functionality

#pragma comment(lib, "Shlwapi.lib")  // Link with the Shlwapi library
#pragma comment(lib, "gdiplus.lib")  // Link with the GDI+ library

#define WM_TRAYICON (WM_USER + 1)  // Define a message for tray icon events
#define ID_TRAY_EXIT 1001
#define ID_TRAY_ABOUT 1002
#define ID_TRAY_AUTOSTART 1003

using namespace std;
using namespace Gdiplus;
using namespace this_thread;  // Using namespace for std::this_thread

HINSTANCE hInst;
NOTIFYICONDATA nid;
bool autostart = false;

// Function declarations
void ShowContextMenu(HWND hwnd, POINT pt);
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_TRAYICON:
		if (lParam == WM_RBUTTONUP) {  // Detect right-click on tray icon
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);  // Ensure the window is foreground to show menu
			ShowContextMenu(hwnd, pt);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TRAY_EXIT:
			PostQuitMessage(0);  // Exit application
			break;
		case ID_TRAY_ABOUT:
			MessageBox(hwnd, L"Nanachi Applet v0.1.0\n\nCreated by N0rule\n\nhttps://n0rule.is-a.dev/", L"About", MB_OK);
			break;
		case ID_TRAY_AUTOSTART:
			autostart = !autostart;
			MessageBox(hwnd, autostart ? L"Autostart enabled!" : L"Autostart disabled!", L"Autostart", MB_OK);
			// Handle autostart option (register/unregister in startup)
			break;
		}
		break;
	case WM_DESTROY:
		RemoveTrayIcon();  // Clean up tray icon
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void ShowContextMenu(HWND hwnd, POINT pt) {
	HMENU hMenu = CreatePopupMenu();
	if (hMenu) {
		InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_ABOUT, L"About");
		InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_AUTOSTART, autostart ? L"Disable Autostart" : L"Enable Autostart");
		InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, L"Exit");

		// Show the menu
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_RIGHTALIGN, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(hMenu);
	}
}

void AddTrayIcon(HWND hwnd) {
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = 1;  // Unique identifier for the tray icon
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;

	// Load the icon from the application's resources (you can also use a custom icon)
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(101));  // Use the application icon

	wcscpy_s(nid.szTip, L"Nanachi Applet");
	Shell_NotifyIcon(NIM_ADD, &nid);  // Add the icon to the system tray
}

void RemoveTrayIcon() {
	Shell_NotifyIcon(NIM_DELETE, &nid);  // Remove the icon from the system tray
}




// Function to set the background image on the monochrome LCD display
bool SetMonoBackgroundFromFile(const wchar_t* imageName)
{
	// Check if the image file exists
	if (!PathFileExistsW(imageName))
	{
		wstring errorMessage = L"The image file '";
		errorMessage += imageName;
		errorMessage += L"' does not exist.";
		MessageBoxW(NULL, errorMessage.c_str(), L"NanachiApplet Error", MB_OK | MB_ICONERROR);
		ExitProcess(EXIT_FAILURE); // Exit the application with an error
	}

	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	// Load the image from file
	Bitmap* imageMono = Bitmap::FromFile(imageName);
	if (imageMono == nullptr || imageMono->GetWidth() == 0 || imageMono->GetHeight() == 0)
	{
		MessageBoxW(NULL, L"Failed to load the image.", L"NanachiApplet Error", MB_OK | MB_ICONERROR);
		delete imageMono; // Delete the image object if it was created
		GdiplusShutdown(gdiplusToken);
		ExitProcess(EXIT_FAILURE); // Exit the application with an error
	}


	// Check if the image dimensions match the LCD display resolution
	if (imageMono->GetWidth() != LOGI_LCD_MONO_WIDTH || imageMono->GetHeight() != LOGI_LCD_MONO_HEIGHT)
	{
		delete imageMono;
		GdiplusShutdown(gdiplusToken);
		MessageBoxW(NULL, L"Incorrect image dimensions.", L"NanachiApplet Error", MB_OK | MB_ICONERROR);
		ExitProcess(EXIT_FAILURE); // Exit the application with an error
	}

	// Create a vector to store the bitmap data
	vector<BYTE> byteBitmapMono(LOGI_LCD_MONO_WIDTH * LOGI_LCD_MONO_HEIGHT);

	// Lock the bitmap for reading
	Rect rect(0, 0, imageMono->GetWidth(), imageMono->GetHeight());
	BitmapData bitmapData;
	imageMono->LockBits(&rect, ImageLockModeRead, PixelFormat32bppRGB, &bitmapData);

	// Get the bitmap data and convert it to monochrome format
	BYTE* srcData = static_cast<BYTE*>(bitmapData.Scan0);
	int stride = bitmapData.Stride;

	for (int y = 0; y < imageMono->GetHeight(); y++)
	{
		for (int x = 0; x < imageMono->GetWidth(); x++)
		{
			int index = y * stride + x * 4;
			byteBitmapMono[y * imageMono->GetWidth() + x] = srcData[index] >= 128 ? 255 : 0;
		}
	}

	// Unlock the bitmap
	imageMono->UnlockBits(&bitmapData);

	// Clean up the image object
	delete imageMono;

	// Set the monochrome background on the LCD display
	bool success = LogiLcdMonoSetBackground(&byteBitmapMono[0]);

	// Shutdown GDI+
	GdiplusShutdown(gdiplusToken);

	return success;
}

// variable to check if time shoud be visible
bool showTime = true;

// Flag to track if the button is pressed
void getCurrentSystemTime() {
	while (true) {
		if (showTime) {
			SYSTEMTIME st;
			GetLocalTime(&st); // Use GetLocalTime for local time

			// Format the date string
			wchar_t dateString[16];
			swprintf(dateString, 16, L"%02d.%02d.%04d", st.wDay, st.wMonth, st.wYear);
			// Set date on line 0
			LogiLcdMonoSetText(0, dateString);


			// Format the time string
			wchar_t timeString[16];
			swprintf(timeString, 16, L"%d:%02d:%02d", st.wHour % 12 == 0 ? 12 : st.wHour % 12, st.wMinute, st.wSecond);
			// Set time on line 1
			LogiLcdMonoSetText(1, timeString);



			LogiLcdUpdate();
		}
		else {
			// Clear both lines when not showing time
			LogiLcdMonoSetText(0, L"");
			LogiLcdMonoSetText(1, L"");
			LogiLcdUpdate();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// variable to check if text should be visible
bool showText = true;

// Flag to track text button
void SetLCDImage() {
	while (true) {

		bool isBlinking = false;
		bool isLookingLeft = false;
		bool isLookingRight = false;

		// Generate random times for normal and blinking states
		int normalTime = rand() % 14000 + 1000; // Random time between 1 second and 15 seconds for normal face
		int blinkTime = rand() % 750 + 250; // Random time between 250 milliseconds and 1 second for blinking
		int lookTime = rand() % 2000 + 1000; // Random time between 1 second and 3 seconds for looking left/right

											 // Generate a random action
		int action = rand() % 100; // Generate a number between 0 and 99

								   // Determine the action based on the random number
		if (action < 35) { // 0 to 34 (35% chance)
			isBlinking = true;
		}
		else if (action < 55) { // 35 to 54 (20% chance)
			isLookingLeft = true;
		}
		else if (action < 75) { // 55 to 74 (20% chance)
			isLookingRight = true;
		}

		//DEBUG CODE,UNUSED
		//wstring normalTimeStr = to_wstring(normalTime); // Convert the integers to wide strings
		//wstring blinkTimeStr = to_wstring(blinkTime);
		//wstring b1 = to_wstring(isBlinking);
		//wstring l1 = to_wstring(isLookingLeft);
		//wstring r1 = to_wstring(isLookingRight);
		//// Send the strings to SetText
		//LogiLcdMonoSetText(0, &normalTimeStr[0]);
		//LogiLcdMonoSetText(3, &blinkTimeStr[0]);
		//LogiLcdUpdate();
		//LogiLcdMonoSetText(0, &b1[0]);
		//LogiLcdMonoSetText(1, &l1[0]);
		//LogiLcdMonoSetText(2, &r1[0]);
		//LogiLcdUpdate();
		// Display the blinking or normal background
		if (isBlinking) {
			SetMonoBackgroundFromFile(L"res/nanachi_blink.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(blinkTime));
		}
		else if (isLookingLeft) {
			SetMonoBackgroundFromFile(L"res/nanachi_lookleft.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(lookTime));
		}
		else if (isLookingRight) {
			SetMonoBackgroundFromFile(L"res/nanachi_lookright.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(lookTime));
		}
		else {
			SetMonoBackgroundFromFile(L"res/nanachi.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(normalTime));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// Function to handle button press events
void buttonHandler() {
	bool isButtonPressed[4] = { false }; // Array to store button states for buttons 0 to 3
	bool prevButtonState[4] = { false }; // Array to store previous button states

	while (true) {
		// Check each button state

		for (int i = 0; i < 4; ++i) {
			// Get the current state of the button
			bool currentButtonState = false;
			switch (i) {
			case 0:
				currentButtonState = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0);
				break;
			case 1:
				currentButtonState = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_1);
				break;
			case 2:
				currentButtonState = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_2);
				break;
			case 3:
				currentButtonState = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_3);
				break;
			default:
				// Invalid button index
				break;
			}

			// Check if button state has changed
			if (currentButtonState != prevButtonState[i]) {
				// Button state has changed, perform action accordingly
				if (currentButtonState) {
					// Button pressed, perform action
					switch (i) {
					case 0:
						// Action for button 1 
						SetMonoBackgroundFromFile(L"res/nanachi_stare.png");
						break;
					case 1:
						// Action for button 2 
						showTime = !showTime;
						break;
					case 2:
						// Action for button 3 
						showText = !showText;
						break;
					case 3:
						// Action for button 4 
						break;
					default:
						// Invalid button index
						break;
					}
				}
				else {
					// Button released, set default background
					switch (i) {
					case 0:
						// Release action for button 1
						SetMonoBackgroundFromFile(L"res/nanachi.png");
						break;
					case 1:
						// Release action for button 2
						break;
					case 2:
						// Release action for button 3
						break;
					case 3:
						// Release action for button 4
						break;
					default:
						// Invalid button index
						break;
					}
				}
				// Update LCD screen
				LogiLcdUpdate();

				// Update previous button state
				prevButtonState[i] = currentButtonState;
			}

			// Update button state in the array
			isButtonPressed[i] = currentButtonState;
		}

		// Sleep for a short duration to avoid consuming too much CPU
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	srand(time(0)); // Seed the random number generator
	hInst = hInstance;

	wchar_t projectName[] = L"Nanachi Applet";

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = projectName;
	RegisterClass(&wc);

	HWND hwnd = CreateWindow(wc.lpszClassName, projectName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300, 150, NULL, NULL, hInstance, NULL);
	AddTrayIcon(hwnd);

	// Initialize the Logitech LCD library
	if (!LogiLcdInit(L"Nanachi Applet", LOGI_LCD_TYPE_MONO)) {
		MessageBoxW(NULL, L"Failed to find monochrome keyboard.", L"NanachiApplet Error", MB_OK | MB_ICONERROR);
		LogiLcdShutdown();
		return 1;
	}



	thread buttonThread(buttonHandler); // Start a new thread for button handling
	thread CurrentSystemTimeThread(getCurrentSystemTime); // Start a new thread for time handling
	thread SetLCDImageThread(SetLCDImage);
	//thread SetTextLCDThread(SetTextLCD); // Start a new thread for text handling



		// Set some initial text on the LCD display

		// LogiLcdMonoSetText(0, L"");
		// LogiLcdMonoSetText(1, L"Привет");
		// LogiLcdMonoSetText(2, L"N0rule");
		// LogiLcdMonoSetText(3, L"");
	LogiLcdMonoSetText(2, L"Привет");
	LogiLcdMonoSetText(3, L"N0rule");

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	LogiLcdShutdown();
	return 0;
}