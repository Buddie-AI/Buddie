@echo off
setlocal enabledelayedexpansion
set INDIR=%1%
set MERGE=%2%
set AROUT=%3%

echo %INDIR%
echo %MERGE%
echo %AROUT%

set FILES=

for /f "tokens=*" %%i in ('dir /b %INDIR%\*.a') DO SET FILES=!FILES! %INDIR%\%%i

echo %FILES%

%MERGE% --no-rewrite --output %AROUT% %FILES%

