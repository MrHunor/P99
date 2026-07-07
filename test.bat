@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo       P99 Media Encoder/Decoder Automated Test
echo ========================================================
echo.

:: Setup Executable and Paths
set "EXE=main.exe"
set "TESTFILES_DIR=testfiles"
set "CONTENT_DIR=%TESTFILES_DIR%\content"
set "IMAGES_SRC=%TESTFILES_DIR%\imagesOriginal"
set "WAV_SRC=%TESTFILES_DIR%\sound.wav"

:: Setup Temporary Output Paths to avoid mutating source assets
set "OUT_DIR=%TESTFILES_DIR%\output_test"
set "OUT_IMAGES_DIR=%OUT_DIR%\images"
set "OUT_DECODE_DIR=%OUT_DIR%\decoded"

echo [!] Cleaning up and initializing test output directories...
if exist "%OUT_DIR%" rmdir /s /q "%OUT_DIR%"
mkdir "%OUT_DIR%"
mkdir "%OUT_IMAGES_DIR%"
mkdir "%OUT_DECODE_DIR%"

:: Mirror or copy original images into output folder for path validations
xcopy "%IMAGES_SRC%\*" "%OUT_IMAGES_DIR%\" /y /q >nul

echo [+] Setup complete. Starting automated tests with maximum verbosity (-vvvv).
echo.

:: --------------------------------------------------------
:: TEST 1: Image Mode (-m) - Single File Encoding & Decoding
:: --------------------------------------------------------
echo --------------------------------------------------------
echo TEST 1: Single Image Mode (-m)
echo --------------------------------------------------------
echo [1/2] Encoding test.txt into a single image...
%EXE% -m -vvvv encode --into "%OUT_IMAGES_DIR%\10.png" --from "%CONTENT_DIR%\test.txt"
if %errorlevel% neq 0 (
    echo [X] TEST 1 Encode Failed with exit code %errorlevel%
) else (
    echo [^] Encode completed successfully.
)

echo [2/2] Decoding test.txt back from the single image...
%EXE% -m -vvvv decode --modified "%OUT_IMAGES_DIR%\10.png" --into "%OUT_DECODE_DIR%\test_recovered_single.txt"
if %errorlevel% neq 0 (
    echo [X] TEST 1 Decode Failed with exit code %errorlevel%
) else (
    echo [^] Decode completed successfully.
)
echo.

:: --------------------------------------------------------
:: TEST 2: Image Mode (-m) - Folder Encoding & Decoding
:: --------------------------------------------------------
echo --------------------------------------------------------
echo TEST 2: Image Folder Mode (-m)
echo --------------------------------------------------------
echo [1/2] Encoding videoO.mp4 into the image folder...
%EXE% -m -vvvv encode --into "%OUT_IMAGES_DIR%" --from "%CONTENT_DIR%\videoO.mp4"
if %errorlevel% neq 0 (
    echo [X] TEST 2 Encode Failed with exit code %errorlevel%
) else (
    echo [^] Encode completed successfully.
)

echo [2/2] Decoding videoO.mp4 back from the image folder...
%EXE% -m -vvvv decode --modified "%OUT_IMAGES_DIR%" --into "%OUT_DECODE_DIR%\videoO_recovered.mp4"
if %errorlevel% neq 0 (
    echo [X] TEST 2 Decode Failed with exit code %errorlevel%
) else (
    echo [^] Decode completed successfully.
)
echo.

:: --------------------------------------------------------
:: TEST 3: Audio Mode (-a) - WAV File Pipeline
:: --------------------------------------------------------
echo --------------------------------------------------------
echo TEST 3: Audio Mode (-a)
echo --------------------------------------------------------
echo [1/2] Encoding content into sound.wav...
%EXE% -a -vvvv encode --into "%WAV_SRC%" --from "%CONTENT_DIR%\test.txt"
if %errorlevel% neq 0 (
    echo [X] TEST 3 Encode Failed with exit code %errorlevel%
) else (
    echo [^] Encode completed successfully.
)

echo [2/2] Decoding content out of sound.wav...
%EXE% -a -vvvv decode --modified "%WAV_SRC%" --into "%OUT_DECODE_DIR%\test_recovered_audio.txt"
if %errorlevel% neq 0 (
    echo [X] TEST 3 Decode Failed with exit code %errorlevel%
) else (
    echo [^] Decode completed successfully.
)
echo.

:: --------------------------------------------------------
:: TEST 4: Error Handling & Help System Validation
:: --------------------------------------------------------
echo --------------------------------------------------------
echo TEST 4: Parameter Validation and Error States
echo --------------------------------------------------------
echo [1/2] Testing validation without missing media type group option (Should show error):
%EXE% encode --into "%OUT_IMAGES_DIR%" --from "%CONTENT_DIR%\test.txt" 2>nul
echo Exit code: %errorlevel% (Expected non-zero error)

echo [2/2] Displaying global expanded help:
%EXE% --help-all > "%OUT_DIR%\help_dump.txt"
echo [^] Global help output dumped safely to %OUT_DIR%\help_dump.txt
echo.

echo ========================================================
echo All tests execution sequence finished.
echo Check the '%OUT_DECODE_DIR%' folder for outputs.
echo ========================================================
pause