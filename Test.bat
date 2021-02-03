setlocal
echo off

rem this must come early because of the 'no build' possibility
set DEVTOOL_OPTIONS=-AutoRun -NoEnvFile -Minimize -q -lf5=Sim.log

rem FOR ASH WARE INTERNAL TESTING ONLY!
set DEVTOOL=%TEST_BED_BIN_DIR%\DevTool.exe
if exist %DEVTOOL% goto FOUND_DEVTOOL

rem FOR ASH WARE INTERNAL TESTING ONLY!
set DEVTOOL=..\..\..\..\Builds\DevTool.exe
if exist %DEVTOOL% goto FOUND_DEVTOOL

rem During installation of the ASH WARE 
rem environment variable, 'DEV_TOOL_FULL_SYS_BIN', is set to point
rem to the latest installed version
set DEVTOOL=%DEV_TOOL_FULL_SYS_BIN%\FullSystemDevTool.exe
if exist %DEVTOOL% goto CheckPathingSuccess

echo .
echo *************************************************
echo It appears the  'DEV_TOOL_FULL_SYS_BIN'  environment variable 
echo is not set, or not set correctly.
echo DEV_TOOL_FULL_SYS_BIN = %DEV_TOOL_FULL_SYS_BIN%
echo Correct this problem in order to build this demo.
echo *************************************************
pause
echo .
goto errors

:FOUND_DEVTOOL

set DEVTOOL_OPTIONS=-p=Project.FullSysIdeProj -AutoBuild -Autorun -IAcceptLicense -NoEnvFile -Minimize -lf5=Sim.Log -q

echo .
echo DevTool:         %DEVTOOL%
echo DEVTOOL_OPTIONS: %DEVTOOL_OPTIONS%
echo CmdLineArgs:     %1  %2  %3  %4
echo .


echo ============================================================
echo Running QD system test
echo ============================================================
del Sim.log
%DEVTOOL% %DEVTOOL_OPTIONS% %1 %2 %3 %4
if  %ERRORLEVEL% NEQ 0 ( goto errors )


echo .
echo All QD System Tests Pass


goto end
:errors
echo *************************************************
echo        YIKES, WE GOT ERRORS!!
echo *************************************************
exit /b -1
:end
exit /b 0
