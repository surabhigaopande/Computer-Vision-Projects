The file VehicleDetetcion.cpp contains the code to count cars using the Region based motion detction approach.
It uses the Intel OpenCV libraries and requires input from a video file.
The installation procedure is as follows:
1.  Install Code::Blocks Code::Blocks is an IDE (integrated development environment). 
Head to their website and download the latest version (codeblocks-10.05-setup.exe) http://www.codeblocks.org/downloads/binaries
Install it to the default location
When the installer finished click yes to run Code::Blocks then go to Settings -> Compiler and Debugger Under the Toolchain Executables select GNU GCC Compiler from the drop down and then press AutoDetect verify that Code::Blocks has found MinGW
If you like now might be a good time to test your Code::Blocks and MinGW setup with a simple Hello World C++ program.

2. Install OpenCV OpenCV is a library of Computer Vision functions. Head to their website and download the latest version (2.4.2 for Windows) http://opencv.org/downloads.html
Click on the OpenCV-2.4.2.exe and choose C:\ as the extract directory
Add OpenCV to the system path C:\opencv\build\x86\mingw\bin (use the same process as above)
Note: Add x86 binaries regardless of your system type (32bit or 64bit) because minGW is 32bit.
Verify that both MinGW and OpenCV are in your system path Make sure you restart Code::Blocks before continuing if you have it open.

3. Configuring Code::Blocks with OpenCV Make a new Code::Blocks Project.
Now run this VehicleDetection.cpp program to test that the install has worked.
