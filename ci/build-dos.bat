@echo off

REM we always must return error code 0, since gitlab runner isn't able
REM today detecting a failure of a command.
REM we check afterwards within the job script for an existing build-output
REM and decide failing or not.

bcpp /m bcm.prj
if %errorlevel% == 0 goto success

:fail
echo "build failed!"
exit /b 0

:success
echo "build succeed!"
exit /b 0

