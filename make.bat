@echo off


SET BINARYFILE=gunzip
SET PATH_RELEASE=build\usr\share\%BINARYFILE%\
SET ORICUTRON="..\..\..\oricutron\"
set VERSION=1.0.1
SET ORIGIN_PATH=%CD%
echo %osdk%
SET PATH=%PATH%


mkdir build\bin\

IF NOT EXIST build\usr\share\ipkg mkdir build\usr\share\ipkg      
IF NOT EXIST build\usr\share\doc mkdir build\usr\share\doc

echo #define VERSION "%VERSION%" > src\version.h

rem  123456789012345678
del src\ipkg\%BINARYFILE%.csv
echo | set /p dummyName=gunzip    1.0.0  gzip files        > src\ipkg\%BINARYFILE%.csv

copy README.md build\usr\share\doc\%BINARYFILE%

copy  src\ipkg\%BINARYFILE%.csv build\usr\share\ipkg\

cl65 -o build\bin\%BINARYFILE% -ttelestrat src\%BINARYFILE%.c 

rem this next line create release
IF "%1"=="NORUN" GOTO End
copy build\bin\%BINARYFILE% %ORICUTRON%\usbdrive\bin
rem copy src\man\%BINARYFILE%.hlp %ORICUTRON%\usbdrive\usr\share\man\
rem copy README.md %ORICUTRON%\usbdrive\usr\share\%BINARYFILE%\
rem copy src\ipkg\%BINARYFILE%.csv %ORICUTRON%\usbdrive\usr\share\ipkg\

cd %ORICUTRON%
OricutronV6 -mt 

cd %ORIGIN_PATH%
:End


:Release