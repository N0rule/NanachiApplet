#include <vector>
#include "LogitechLCDLib.h"  // Library for interacting with Logitech LCD displays
#include <gdiplus.h>  // Library for working with graphics and images
#include <Windows.h>
#include <Shlwapi.h>  // Library for path handling
#include <string>
#include <thread>  // Library for creating and managing threads
#pragma comment(lib, "Shlwapi.lib")  // Link with the Shlwapi library
#pragma comment(lib, "gdiplus.lib")  // Link with the GDI+ library

using namespace std;
using namespace Gdiplus;
using namespace this_thread;  // Using namespace for std::this_thread

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

// Flag to track if the button is pressed


// Function to handle button press events
void buttonHandler() {
	bool buttonPressed = false;
	while (true) {
		// Check if the button is currently pressed
		bool isCurrentlyPressed = LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0);

		// Button has been pressed
		if (isCurrentlyPressed && !buttonPressed) {
			buttonPressed = true;
			SetMonoBackgroundFromFile(L"res/background_stare.png");
			LogiLcdUpdate();
		}
		// Button has been released
		else if (!isCurrentlyPressed && buttonPressed) {
			buttonPressed = false;
			SetMonoBackgroundFromFile(L"res/background.png");
			LogiLcdUpdate();
		}

		// Sleep for a short duration to avoid consuming too much CPU
		sleep_for(chrono::milliseconds(100));
	}
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	srand(time(0)); // Seed the random number generator

	wchar_t projectName[] = L"Nanachi Applet";
	// Initialize the Logitech LCD library
	if (!LogiLcdInit(L"Nanachi Applet", LOGI_LCD_TYPE_MONO))
	{
		MessageBoxW(NULL, L"Failed to find monochrome keyboard.", L"NanachiApplet Error", MB_OK | MB_ICONERROR);
		LogiLcdShutdown();
		return 1;
	}

	// Set some initial text on the LCD display
	LogiLcdMonoSetText(0, L"");
	LogiLcdMonoSetText(1, L"Привет");
	LogiLcdMonoSetText(2, L"N0rule");
	LogiLcdMonoSetText(3, L"");

	bool isBlinking = false;
	thread buttonThread(buttonHandler); // Start a new thread for button handling

	while (true) {
		// Generate random times for normal and blinking states
		int normalTime = rand() % 14000 + 1000; // Random time between 1 second and 15 seconds for normal face
		int blinkTime = rand() % 750 + 250; // Random time between 250 milliseconds and 1 second for blinking

		//DEBUG CODE,UNUSED
		//wstring normalTimeStr = to_wstring(normalTime); // Convert the integers to wide strings
		//wstring blinkTimeStr = to_wstring(blinkTime);

		//// Send the strings to SetText
		//LogiLcdMonoSetText(0, &normalTimeStr[0]);
		//LogiLcdMonoSetText(3, &blinkTimeStr[0]);
		//LogiLcdUpdate();

		// Display the blinking or normal background
		if (isBlinking) {
			SetMonoBackgroundFromFile(L"res/background_blink.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(blinkTime));
		}
		else {
			SetMonoBackgroundFromFile(L"res/background.png");
			LogiLcdUpdate();
			sleep_for(chrono::milliseconds(normalTime));
		}

		// Toggle the blinking state
		isBlinking = !isBlinking;
	}

	LogiLcdShutdown();
	return 0;
}