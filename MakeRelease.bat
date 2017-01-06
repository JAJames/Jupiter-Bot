@ECHO OFF
if NOT "%1" == "-scr" ECHO Copyright (C) 2014-2017 Jessica James. All rights reserved.
ECHO.

SET Platform=Win32
SET NoArgs=False

if "%1" == "" SET NoArgs=True

:ParseParams:
if "%1" == "-platform" (
	SET Platform=%2
	SHIFT /1
	GOTO ParseParamsCondition
)
if "%1" == "/?" GOTO Help
if "%1" == "-help" GOTO Help
if "%1" == "--help" GOTO Help
if "%1" == "-clean" GOTO CleanUp
if "%1" == "-binary" GOTO BinaryCopy
if "%1" == "-source" GOTO SourceCopy

:ParseParamsCondition:
SHIFT /1
if NOT "%1" == "" GOTO ParseParams
:EndParseParams:

CALL %0 -scr -platform %Platform% -clean
CALL %0 -scr -platform %Platform% -binary
CALL %0 -scr -platform %Platform% -source
if %NoArgs% == "True" GOTO Done
GOTO EOF

:Help:
ECHO This is a convenience script to assist in build release.
ECHO Usage: %0 [-clean/-binary/-source]
ECHO.
ECHO Clean:	Deletes any contents from previous builds.
ECHO Binary:	Copies and ZIPS the essential binaries to run the applicaiton.
ECHO Source:	Copies and ZIPS the source code to run the application.
ECHO.
ECHO If no parameters are given, then all functions will be executed.
ECHO.
ECHO If a parameter other than "-clean", "-binary", or "-source" is given,
ECHO then the program will execute all functions without pausing at the end.
ECHO.
ECHO Program execution order:
ECHO 	Clean
ECHO 	Binary
ECHO 	Source
ECHO.
ECHO Dependencies:
ECHO 	COPY, ROBOCOPY, DEL, RMDIR, PAUSE, ECHO, GOTO, IF, CALL
GOTO EOF

:CleanUp:
RMDIR /S /Q "..\Jupiter Bot Binaries"
RMDIR /S /Q "..\Jupiter Bot Source"
DEL /F /Q "..\Jupiter Bot Binaries.zip"
DEL /F /Q "..\Jupiter Bot.zip"
GOTO EOF

:BinaryCopy:
ROBOCOPY "%Platform%\Release\\" "..\Jupiter Bot Binaries\\" *.dll *.exe /S /xf Tester.exe
ROBOCOPY "Configs\\" "..\Jupiter Bot Binaries\Configs\\" *
ROBOCOPY ".\\" "..\Jupiter Bot Binaries\\" *.ini *.txt LICENSE
"C:\Program Files\WinRAR\WinRAR.exe" a -r "..\Jupiter Bot Binaries.zip" "..\Jupiter Bot Binaries"
GOTO EOF

:SourceCopy:
ROBOCOPY ".\\" "..\Jupiter Bot Source\\" *.* /S /XD Win32 x64 .*
ROBOCOPY "%Platform%\Release\\" "..\Jupiter Bot Source\%Platform%\Release\\" *.dll *.exe /S
ROBOCOPY "Configs\\" "..\Jupiter Bot Source\Configs\\" *
ROBOCOPY ".\\" "..\Jupiter Bot Source\\" *.ini *.txt LICENSE
"C:\Program Files\WinRAR\WinRAR.exe" a -r "..\Jupiter Bot.zip" "..\Jupiter Bot Source"
GOTO EOF

:Done:
ECHO Operations complete.
PAUSE
GOTO EOF

:EOF:
