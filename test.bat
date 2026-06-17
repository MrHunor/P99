@echo off
echo This will not currently work 
pause
exit /b 1
setlocal enabledelayedexpansion

echo ============================================
echo  STEGANOGRAPHY TOOL - AUTOMATED TEST
echo ============================================

:: ----- 1. Choose the executable (main.exe or P99.exe) -----
set "EXE=main.exe"
if exist "P99.exe" set "EXE=P99.exe"
if not exist "%EXE%" (
    echo [INFO] %EXE% not found. Attempting to compile...
    where g++ >nul 2>nul
    if %errorlevel% neq 0 (
        echo [ERROR] g++ not found in PATH. Install MinGW or add g++ to PATH.
        pause
        exit /b 1
    )
    g++ main.cpp header.cpp -o main.exe -std=c++17
    if %errorlevel% neq 0 (
        echo [ERROR] Compilation failed.
        pause
        exit /b 1
    )
    set "EXE=main.exe"
    echo [INFO] Compilation successful.
)

:: ----- 2. Clean up previous test leftovers -----
echo [INFO] Cleaning previous test outputs...
if exist "video.mp4" del /f /q "video.mp4"
if exist "testfiles\content\video_original.mp4" (
    echo [INFO] Restoring original video.mp4 from backup...
    move /y "testfiles\content\video_original.mp4" "testfiles\content\video.mp4" >nul
)
if exist "testfiles\imagesOriginalM" rd /s /q "testfiles\imagesOriginalM"

:: ----- 3. Verify input files -----
if not exist "testfiles\imagesOriginal\" (
    echo [ERROR] Folder testfiles\imagesOriginal\ missing.
    pause
    exit /b 1
)
if not exist "testfiles\content\video.mp4" (
    echo [ERROR] testfiles\content\video.mp4 not found.
    pause
    exit /b 1
)

:: ----- 4. Encode -----
echo.
echo [STEP 1] Encoding video.mp4 into images...
%EXE% encode testfiles\imagesOriginal testfiles\content\video.mp4
if %errorlevel% neq 0 (
    echo [ERROR] Encoding failed.
    pause
    exit /b 1
)
echo [INFO] Encoding finished. Modified images are in testfiles\imagesOriginalM\

:: ----- 5. Rename original video to avoid overwriting during decode -----
echo.
echo [STEP 2] Renaming original video to video_original.mp4...
move /y "testfiles\content\video.mp4" "testfiles\content\video_original.mp4" >nul
if %errorlevel% neq 0 (
    echo [ERROR] Could not rename original video.
    pause
    exit /b 1
)

:: ----- 6. Decode -----
echo.
echo [STEP 3] Decoding hidden data...
%EXE% decode testfiles\imagesOriginal testfiles\imagesOriginalM
if %errorlevel% neq 0 (
    echo [ERROR] Decoding failed.
    move /y "testfiles\content\video_original.mp4" "testfiles\content\video.mp4" >nul
    pause
    exit /b 1
)
echo [INFO] Decoding finished. Output file is testfiles\content\video.mp4

:: ----- 7. Compare hashes -----
echo.
echo [STEP 4] Comparing SHA-256 hashes...
certutil -hashfile "testfiles\content\video_original.mp4" SHA256 > orig_hash.txt
certutil -hashfile "testfiles\content\video.mp4" SHA256 > decoded_hash.txt

:: Extract hash values (second line of each file)
for /f "skip=1 delims=" %%a in (orig_hash.txt) do set "orig_hash=%%a" & goto :got_orig
:got_orig
for /f "skip=1 delims=" %%a in (decoded_hash.txt) do set "decoded_hash=%%a" & goto :got_decoded
:got_decoded

:: Trim any spaces
set orig_hash=%orig_hash: =%
set decoded_hash=%decoded_hash: =%

echo Original  : %orig_hash%
echo Decoded   : %decoded_hash%

if /i "%orig_hash%"=="%decoded_hash%" (
    echo [SUCCESS] Hashes match - data recovered perfectly.
) else (
    echo [FAIL] Hashes do NOT match.
)

:: ----- 8. Clean up temporary files and restore original video name -----
del /f /q orig_hash.txt decoded_hash.txt
move /y "testfiles\content\video_original.mp4" "testfiles\content\video.mp4" >nul
echo [INFO] Original video.mp4 restored.

echo.
echo Test finished.
pause