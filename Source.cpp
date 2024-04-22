// SetMonoBackgroundFromFile
//
// Loads a .bmp file from the specified file path and converts it to a monochrome
// image that can be sent to the Logitech LCD device. The function returns true on
// success and false otherwise.

#include <iostream>
#include <vector>
#include "LogitechLCDLib.h"
#include <gdiplus.h>
#include <thread> // Include for this_thread::sleep_for

#pragma comment(lib, "gdiplus.lib")

using namespace std;
using namespace Gdiplus;

const wchar_t* imageName = L"res/background.png";


bool SetMonoBackgroundFromFile(const wchar_t* imageName) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    Bitmap* imageMono = Bitmap::FromFile(imageName);
    if (imageMono == nullptr)
        return false;

    // Check that the loaded image has the correct dimensions
    if (imageMono->GetWidth() != LOGI_LCD_MONO_WIDTH || imageMono->GetHeight() != LOGI_LCD_MONO_HEIGHT) {
        delete imageMono;
        GdiplusShutdown(gdiplusToken);
        return false;
    }

    vector<BYTE> byteBitmapMono(LOGI_LCD_MONO_WIDTH * LOGI_LCD_MONO_HEIGHT);
    Rect rect(0, 0, imageMono->GetWidth(), imageMono->GetHeight());
    BitmapData bitmapData;
    imageMono->LockBits(&rect, ImageLockModeRead, PixelFormat32bppRGB, &bitmapData);

    BYTE* srcData = static_cast<BYTE*>(bitmapData.Scan0);
    int stride = bitmapData.Stride;

    for (int y = 0; y < imageMono->GetHeight(); y++) {
        for (int x = 0; x < imageMono->GetWidth(); x++) {
            int index = y * stride + x * 4;
            // Convert the color value from RGB to monochrome
            byteBitmapMono[y * imageMono->GetWidth() + x] = srcData[index] >= 128 ? 255 : 0;
        }
    }

    imageMono->UnlockBits(&bitmapData);
    delete imageMono;

    bool success = LogiLcdMonoSetBackground(&byteBitmapMono[0]);
    GdiplusShutdown(gdiplusToken);
    return success;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    wchar_t projectName[] = L"Nanachi Applet";
    // Initialize the Logitech LCD device with the specified project name and
    // a monochrome display type
    if (!LogiLcdInit(projectName, LOGI_LCD_TYPE_MONO)) {
        wcerr << "Failed to find monocrome keyboard." << endl;
        // Shut down the applet if there was an error
        LogiLcdShutdown();
        return 1;
    }
    else
        wcout << "Keyboard found!" << endl;

    wchar_t firstString[] = L"Привет";
    if (LogiLcdMonoSetText(1, firstString))
        wcout << "1 text is Written!" << endl;

    wchar_t secondString[] = L"N0rule";
    if (LogiLcdMonoSetText(2, secondString))
        wcout << "2 text is Written!" << endl;

    // Load the image from file and set it as the background
    if (!SetMonoBackgroundFromFile(imageName)) {
        wcerr << "Failed to set background image." << endl;
        // Shut down the applet if there was an error
        LogiLcdShutdown();
        return 1;
    }
    else
        wcout << imageName << " " << "as background Set!" << endl;

    while (true) {
        // Update the LCD display
        LogiLcdUpdate();

        // Add a delay to reduce CPU usage
        this_thread::sleep_for(chrono::hours(24)); // Adjust delay as needed
    }

    // This code will never be reached, but included for completeness
    LogiLcdShutdown();
    return 0;
}