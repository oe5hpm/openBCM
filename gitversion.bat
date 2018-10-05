@echo off
cd /d "%~dp0"

"C:\Programme\Git\bin\git.exe" describe --tags --abbrev=4 --always --dirty > gitver.h
set /p VAR=<gitver.h
echo #define GITVERSION ^"%VAR%^" > gitver.h

copy bcm32.rc.template bcm32.rc /y
ext\_sed -i "s/_productversion_/%VAR%/g" bcm32.rc
del sed* /f

exit /b 0
