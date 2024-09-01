Logitech Gaming Lcd SDK
Copyright (C) 2014 Logitech Inc. All Rights Reserved


Introduction
--------------------------------------------------------------------------

This package enables developers to easily add support in their games
for lcd screen on compatible Logitech devices.


Contents of the package
--------------------------------------------------------------------------

- Logitech Gaming Lcd SDK header, libs 32 and 64 bit
- Logitech Gaming Lcd SDK game engines wrapper dll
- Demo executable
- Documentation
- Sample program using the SDK


The environment for use of the package
--------------------------------------------------------------------------

1. Visual Studio 2012 to build and run the sample program

List of currently supported devices
--------------------------------------------------------------------------

Devices

- G19 / G19s - 320x240 Full RGBA
- G510 / G510s - 160x43 Monochrome
- G13 - 160x43 Monochrome
- G15 v1 - 160x43 Monochrome
- G15 v2 - 160x43 Monochrome


Disclaimer
--------------------------------------------------------------------------

This is work in progress. If you find anything wrong with either
documentation or code, please let us know so we can improve on it.


Where to start
--------------------------------------------------------------------------

For a demo program to change lighting on devices:

Execute Demo/LCDDemo.exe.

Or:

1. Go to Samples/LCDDemo folder and open the project in
   Visual Studio

2. Compile and run.

3. Plug in one or multiple compatible devices at any time.


To implement game controller support in your game:

1. Include the following header file in your game:

- Include/LogitechLcdLib.h

2. Include the following library in your game:

- Lib\x86\LogitechLcdLib.lib
- Lib\x64\LogitechLcdLib.lib

3. Read and follow instructions from Doc/LogitechGamingLCDSDK.pdf


For questions/problems/suggestions email to:
tpigliucci@logitech.com
