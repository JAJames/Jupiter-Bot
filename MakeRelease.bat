@ECHO OFF
if NOT "%2" == "-scr" ECHO Copyright (C) 2014 Justin James. All rights reserved.
ECHO.

if "%1" == "/?" GOTO Help
if "%1" == "-help" GOTO Help
if "%1" == "--help" GOTO Help
if "%1" == "-clean" GOTO CleanUp
if "%1" == "-binary" GOTO BinaryCopy
if "%1" == "-source" GOTO SourceCopy
CALL %0 -clean -scr
CALL %0 -binary -scr
CALL %0 -source -scr
if "%1" == "" GOTO Done
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
ROBOCOPY "Release\\" "..\Jupiter Bot Binaries\\" *.dll *.exe ReadMe.txt /S
COPY /Y "Release\Example Config.ini" "..\Jupiter Bot Binaries\Config.ini"
COPY /Y "Release\Translations.ini" "..\Jupiter Bot Binaries\Translations.ini"
COPY /Y "Release\Mods.ini" "..\Jupiter Bot Binaries\Mods.ini"
COPY /Y "Release\RenXGameCommands.ini" "..\Jupiter Bot Binaries\RenXGameCommands.ini"
"C:\Program Files\WinRAR\WinRAR.exe" a -r "..\Jupiter Bot Binaries.zip" "..\Jupiter Bot Binaries"
GOTO EOF

:SourceCopy:
ROBOCOPY ".\\" "..\Jupiter Bot Source\\" *.* /S /XD Release
ROBOCOPY "Release\\" "..\Jupiter Bot Source\Release\\" *.dll *.exe ReadMe.txt /S
COPY /Y "Release\Example Config.ini" "..\Jupiter Bot Source\Release\Config.ini"
COPY /Y "Release\Translations.ini" "..\Jupiter Bot Source\Release\Translations.ini"
COPY /Y "Release\Mods.ini" "..\Jupiter Bot Source\Release\Mods.ini"
COPY /Y "Release\RenXGameCommands.ini" "..\Jupiter Bot Source\Release\RenXGameCommands.ini"
"C:\Program Files\WinRAR\WinRAR.exe" a -r "..\Jupiter Bot.zip" "..\Jupiter Bot Source"
GOTO EOF

:Done:
ECHO Operations complete.
PAUSE
GOTO EOF

:EOF: