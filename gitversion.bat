@echo off
"C:\Programme\Git\bin\git.exe" describe --tags --abbrev=4 --always --dirty > gitversion.h
set /p VAR=<gitversion.h
echo #define GITVERSION ^"%VAR%^" > gitversion.h

copy bcm32.rc.template bcm32.rc /y
ext\_sed -i "s/_productversion_/%VAR%/g" bcm32.rc
del sed* /f
