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
						break;
					case 2:
						// Action for button 3 
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

	thread buttonThread(buttonHandler); // Start a new thread for button handling

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
	}
	LogiLcdShutdown();
	return 0;
}