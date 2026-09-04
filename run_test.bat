@echo off
setlocal EnableDelayedExpansion

:: -----------------------------------------------------------
:: ADAM Fastchess Automated Testing Runner
:: -----------------------------------------------------------

set "SCRIPT_DIR=%~dp0"
set "FASTCHESS=C:\Users\sriva\OneDrive\Desktop\fastchess-windows-x86-64\fastchess.exe"
set "BOOK=C:\Users\sriva\OneDrive\Desktop\fastchess-windows-x86-64\app\tests\data\openings.epd"
set "DEV_ENGINE=%SCRIPT_DIR%ADAM.exe"
set "BASE_ENGINE=%SCRIPT_DIR%ADAM_base.exe"
set "PGN_OUT=%SCRIPT_DIR%test_results.pgn"
set "CONCURRENCY=6"

echo ===================================================
echo           ADAM CHESS ENGINE - TEST RUNNER
echo ===================================================
echo Fastchess : %FASTCHESS%
echo Book      : %BOOK%
echo Dev Build : %DEV_ENGINE%
echo Base Build: %BASE_ENGINE%
echo Threads   : %CONCURRENCY% concurrent games
echo ===================================================
echo.

if not exist "%FASTCHESS%" (
    echo [ERROR] fastchess.exe not found at:
    echo "%FASTCHESS%"
    echo Please check the path and try again.
    pause
    exit /b 1
)

if not exist "%DEV_ENGINE%" (
    echo [ERROR] ADAM.exe not found. Please compile the engine first.
    pause
    exit /b 1
)

:MENU
echo Select a testing mode:
echo [1] Quick Sanity Test  (20 rounds / 40 games, tc=5+0.05, ~1-2 min)
echo [2] Standard Match     (100 rounds / 200 games, tc=10+0.1, ~10-15 min)
echo [3] SPRT Test          (elo0=0 elo1=5, auto-stops when +5 Elo is proven)
echo [4] Save current ADAM.exe as new ADAM_base.exe
echo [5] Exit
echo.
set /p CHOICE="Enter choice [1-5]: "

if "%CHOICE%"=="1" goto QUICK
if "%CHOICE%"=="2" goto STANDARD
if "%CHOICE%"=="3" goto SPRT
if "%CHOICE%"=="4" goto UPDATE_BASE
if "%CHOICE%"=="5" exit /b 0
echo Invalid choice, try again.
echo.
goto MENU

:CHECK_BASE
if not exist "%BASE_ENGINE%" (
    echo.
    echo [WARNING] ADAM_base.exe not found!
    echo Creating ADAM_base.exe from current ADAM.exe as baseline...
    copy "%DEV_ENGINE%" "%BASE_ENGINE%" >nul
    echo Done! Now you can modify and recompile ADAM.exe to test against this baseline.
    echo.
)
goto :eof

:QUICK
call :CHECK_BASE
echo.
echo Starting Quick Sanity Test (20 rounds, tc=5+0.05)...
"%FASTCHESS%" ^
  -engine cmd="%DEV_ENGINE%" name=ADAM_dev option.Hash=32 ^
  -engine cmd="%BASE_ENGINE%" name=ADAM_base option.Hash=32 ^
  -each tc=5+0.05 ^
  -rounds 20 -repeat ^
  -concurrency %CONCURRENCY% ^
  -openings file="%BOOK%" format=epd order=random ^
  -draw movenumber=35 movecount=8 score=10 ^
  -resign movecount=4 score=600 ^
  -pgnout file="%PGN_OUT%"
pause
goto MENU

:STANDARD
call :CHECK_BASE
echo.
echo Starting Standard Match (100 rounds, tc=10+0.1)...
"%FASTCHESS%" ^
  -engine cmd="%DEV_ENGINE%" name=ADAM_dev option.Hash=32 ^
  -engine cmd="%BASE_ENGINE%" name=ADAM_base option.Hash=32 ^
  -each tc=10+0.1 ^
  -rounds 100 -repeat ^
  -concurrency %CONCURRENCY% ^
  -openings file="%BOOK%" format=epd order=random ^
  -draw movenumber=35 movecount=8 score=10 ^
  -resign movecount=4 score=600 ^
  -pgnout file="%PGN_OUT%"
pause
goto MENU

:SPRT
call :CHECK_BASE
echo.
echo Starting SPRT Test (elo0=0, elo1=5, tc=8+0.08)...
"%FASTCHESS%" ^
  -engine cmd="%DEV_ENGINE%" name=ADAM_dev option.Hash=32 ^
  -engine cmd="%BASE_ENGINE%" name=ADAM_base option.Hash=32 ^
  -each tc=8+0.08 ^
  -sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 ^
  -rounds 2000 -repeat ^
  -concurrency %CONCURRENCY% ^
  -openings file="%BOOK%" format=epd order=random ^
  -draw movenumber=35 movecount=8 score=10 ^
  -resign movecount=4 score=600 ^
  -pgnout file="%PGN_OUT%"
pause
goto MENU

:UPDATE_BASE
echo.
copy /y "%DEV_ENGINE%" "%BASE_ENGINE%"
echo [SUCCESS] Current ADAM.exe copied to ADAM_base.exe!
echo.
pause
goto MENU
