# Group 75 CE/CZ4031 Database System Principles Project 01

### Windows

Ensure MinGW is installed

To install

- Download 7-zip exe from this link https://nuwen.net/files/mingw/mingw-18.0-without-git.exe
- Select extract folder "c:/"
- Go to "Control Panel\All Control Panel Items\System"
- Click on Advanced system settings on the left panel
- Click on Environment Variables
- Select Path and click Edit
- Click New to add new row in the Path and type in "C:\MinGW\bin"
- Click OK on all of the open pop up windows to confirm setting updates.

Verify installation

- Press Windows+R and run "CMD"
- Type in "g++ --version" into the command prompt and press enter.
- Something like below message should be printed out

g++ (GCC) 11.2.0
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions. There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Visual Studio Code

- Install Code Runner extension, by Jun Han
- Install C/C++ extension, by Microsoft

## Running the code

Execute this command in terminal
cd "PATH OF PROJECT" ; if ($?) { g++ main.cpp diskStorage.cpp table_printer.cpp -o main  } ; if ($?) { .\main }

or
open CMD or powershell
cd "PATH OF PROJECT" ;
.\main.exe
