@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo               P99 TEST PIPELINE
echo ===================================================

:: Configuration
set "EXE=main.exe"
set "SRC_TXT=testfiles\content\test.txt"
set "SRC_VID=testfiles\content\video.mp4"
set "IMAGES_ORIG_DIR=testfiles\imagesOriginal"
set "SINGLE_ORIG_IMG=%IMAGES_ORIG_DIR%\1.png"

set "TEMP_TXT=temp_test.txt"
set "TEMP_VID=temp_video.mp4"
set "SINGLE_OUT_IMG=output.png"
set "FOLDER_OUT_DIR=%IMAGES_ORIG_DIR%M"

:: Initial Cleanup
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"

echo.
echo --- TEST 1: Single Image Mode (TXT) ---
copy "%SRC_TXT%" "%TEMP_TXT%" >nul

echo [CMD] %EXE% -v encode -i "%SINGLE_ORIG_IMG%" -f "%TEMP_TXT%"
"%EXE%" -v encode -i "%SINGLE_ORIG_IMG%" -f "%TEMP_TXT%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

del /q "%TEMP_TXT%"

echo [CMD] %EXE% -v decode -m "%SINGLE_OUT_IMG%" -f "%SINGLE_ORIG_IMG%"
"%EXE%" -v decode -m "%SINGLE_OUT_IMG%" -f "%SINGLE_ORIG_IMG%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

if not exist "%TEMP_TXT%" (
    echo [ERROR] Expected output file '%TEMP_TXT%' was not generated.
    goto cleanup_fail
)

:: Extract and compare SHA-256 hashes
set "HASH_SRC="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%SRC_TXT%" SHA256 2^>nul') do if not defined HASH_SRC set "HASH_SRC=%%A"
set "HASH_SRC=%HASH_SRC: =%"

set "HASH_OUT="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%TEMP_TXT%" SHA256 2^>nul') do if not defined HASH_OUT set "HASH_OUT=%%A"
set "HASH_OUT=%HASH_OUT: =%"

echo [HASH] Src: %HASH_SRC%
echo [HASH] Out: %HASH_OUT%

if /i "%HASH_SRC%" neq "%HASH_OUT%" (
    echo [FAIL] Hash mismatch detected.
    goto cleanup_fail
)
echo [OK] Test 1 verified.

echo.
echo --- TEST 2: Folder Mode (MP4) ---
copy "%SRC_VID%" "%TEMP_VID%" >nul

echo [CMD] %EXE% -v encode -i "%IMAGES_ORIG_DIR%" -f "%TEMP_VID%"
"%EXE%" -v encode -i "%IMAGES_ORIG_DIR%" -f "%TEMP_VID%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

del /q "%TEMP_VID%"

echo [CMD] %EXE% -v decode -m "%FOLDER_OUT_DIR%" -f "%IMAGES_ORIG_DIR%"
"%EXE%" -v decode -m "%FOLDER_OUT_DIR%" -f "%IMAGES_ORIG_DIR%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

if not exist "%TEMP_VID%" (
    echo [ERROR] Expected output file '%TEMP_VID%' was not generated.
    goto cleanup_fail
)

:: Extract and compare SHA-256 hashes
set "HASH_SRC="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%SRC_VID%" SHA256 2^>nul') do if not defined HASH_SRC set "HASH_SRC=%%A"
set "HASH_SRC=%HASH_SRC: =%"

set "HASH_OUT="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%TEMP_VID%" SHA256 2^>nul') do if not defined HASH_OUT set "HASH_OUT=%%A"
set "HASH_OUT=%HASH_OUT: =%"

echo [HASH] Src: %HASH_SRC%
echo [HASH] Out: %HASH_OUT%

if /i "%HASH_SRC%" neq "%HASH_OUT%" (
    echo [FAIL] Hash mismatch detected.
    goto cleanup_fail
)
echo [OK] Test 2 verified.

:cleanup_pass
echo.
echo --- CLEANUP ---
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"
echo ===================================================
echo STATUS: SUCCESS
echo ===================================================
exit /b 0

:cleanup_fail
echo.
echo --- CLEANUP (FAIL HINT) ---
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"
echo ===================================================
echo STATUS: FAILED
echo ===================================================
exit /b 1