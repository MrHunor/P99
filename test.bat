@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo                 P99 TEST PIPELINE
echo ===================================================

:: Configuration
set "EXE=main.exe"
set "SRC_TXT=testfiles\content\test.txt"
set "SRC_VID=testfiles\content\video.mp4"
set "SRC_AUDIO=testfiles\sound.wav"

set "IMAGES_ORIG_DIR=testfiles\imagesOriginal"
set "SINGLE_ORIG_IMG=%IMAGES_ORIG_DIR%\1.png"

:: Temporary decoded payload outputs
set "TEMP_TXT=temp_test.txt"
set "TEMP_VID=temp_video.mp4"
set "TEMP_TXT_AUDIO=temp_audio_test.txt"

:: Real target outputs based on application's naming convention
set "SINGLE_OUT_IMG=output.png"
set "FOLDER_OUT_DIR=%IMAGES_ORIG_DIR%M"
set "SINGLE_OUT_AUDIO=testfiles\soundM.wav"

:: Initial Cleanup to avoid false positives
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%TEMP_TXT_AUDIO%" del /q "%TEMP_TXT_AUDIO%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"
if exist "%SINGLE_OUT_AUDIO%" del /q "%SINGLE_OUT_AUDIO%"

echo.
echo --- TEST 1: Single Image Mode (TXT) ---
copy "%SRC_TXT%" "%TEMP_TXT%" >nul

echo [CMD] %EXE% -vvvv -m encode --into "%SINGLE_ORIG_IMG%" --from "%TEMP_TXT%"
"%EXE%" -vvvv -m encode --into "%SINGLE_ORIG_IMG%" --from "%TEMP_TXT%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

del /q "%TEMP_TXT%"

echo [CMD] %EXE% -vvvv -m decode --modified "%SINGLE_OUT_IMG%" --original "%SINGLE_ORIG_IMG%"
"%EXE%" -vvvv -m decode --modified "%SINGLE_OUT_IMG%" --original "%SINGLE_ORIG_IMG%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

if not exist "%TEMP_TXT%" (
    echo [ERROR] Expected output file '%TEMP_TXT%' was not recreated by the decoder.
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

echo [CMD] %EXE% -vvvv -m encode --into "%IMAGES_ORIG_DIR%" --from "%TEMP_VID%"
"%EXE%" -vvvv -m encode --into "%IMAGES_ORIG_DIR%" --from "%TEMP_VID%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

del /q "%TEMP_VID%"

echo [CMD] %EXE% -vvvv -m decode --modified "%FOLDER_OUT_DIR%" --original "%IMAGES_ORIG_DIR%"
"%EXE%" -vvvv -m decode --modified "%FOLDER_OUT_DIR%" --original "%IMAGES_ORIG_DIR%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

if not exist "%TEMP_VID%" (
    echo [ERROR] Expected output file '%TEMP_VID%' was not recreated by the decoder.
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

echo.
echo --- TEST 3: Single Audio Mode (TXT) ---
copy "%SRC_TXT%" "%TEMP_TXT_AUDIO%" >nul

echo [CMD] %EXE% -vvvv -a encode --into "%SRC_AUDIO%" --from "%TEMP_TXT_AUDIO%"
"%EXE%" -vvvv -a encode --into "%SRC_AUDIO%" --from "%TEMP_TXT_AUDIO%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

del /q "%TEMP_TXT_AUDIO%"

echo [CMD] %EXE% -vvvv -a decode --modified "%SINGLE_OUT_AUDIO%" --original "%SRC_AUDIO%"
"%EXE%" -vvvv -a decode --modified "%SINGLE_OUT_AUDIO%" --original "%SRC_AUDIO%"
if %ERRORLEVEL% neq 0 goto cleanup_fail

if not exist "%TEMP_TXT_AUDIO%" (
    echo [ERROR] Expected output file '%TEMP_TXT_AUDIO%' was not recreated by the decoder.
    goto cleanup_fail
)

:: Extract and compare SHA-256 hashes
set "HASH_SRC="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%SRC_TXT%" SHA256 2^>nul') do if not defined HASH_SRC set "HASH_SRC=%%A"
set "HASH_SRC=%HASH_SRC: =%"

set "HASH_OUT="
for /f "skip=1 tokens=*" %%A in ('certutil -hashfile "%TEMP_TXT_AUDIO%" SHA256 2^>nul') do if not defined HASH_OUT set "HASH_OUT=%%A"
set "HASH_OUT=%HASH_OUT: =%"

echo [HASH] Src: %HASH_SRC%
echo [HASH] Out: %HASH_OUT%

if /i "%HASH_SRC%" neq "%HASH_OUT%" (
    echo [FAIL] Hash mismatch detected.
    goto cleanup_fail
)
echo [OK] Test 3 verified.

:cleanup_pass
echo.
echo --- CLEANUP ---
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%TEMP_TXT_AUDIO%" del /q "%TEMP_TXT_AUDIO%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"
if exist "%SINGLE_OUT_AUDIO%" del /q "%SINGLE_OUT_AUDIO%"
echo ===================================================
echo STATUS: SUCCESS
echo ===================================================
exit /b 0

:cleanup_fail
echo.
echo --- CLEANUP (FAIL HINT) ---
if exist "%TEMP_TXT%" del /q "%TEMP_TXT%"
if exist "%TEMP_VID%" del /q "%TEMP_VID%"
if exist "%TEMP_TXT_AUDIO%" del /q "%TEMP_TXT_AUDIO%"
if exist "%SINGLE_OUT_IMG%" del /q "%SINGLE_OUT_IMG%"
if exist "%FOLDER_OUT_DIR%" rmdir /s /q "%FOLDER_OUT_DIR%"
if exist "%SINGLE_OUT_AUDIO%" del /q "%SINGLE_OUT_AUDIO%"
echo ===================================================
echo STATUS: FAILED
echo ===================================================
exit /b 1