# P99 - File to Media File Steganography (AI Generated Overview) © 2026 MrHunor, GPLv3



## Features

 Encode File: Hide confidential or arbitrary files directly within the pixel data of a PNG image, or in the audio samples of a WAV file.

 Decode File: Extract previously hidden files from a modified PNG image or WAV file, by comparing it against the original.

 Folder Mode: Encode/decode into every image in a folder at once, in addition to single-file mode.

 Command Line Interface: Uses `CLI11` for structured arguments and help menus.

 Windows Ready: Includes pre-configured build files for VS Code and an automated test batch script.



## Repository Structure

 `main.cpp` / `header.cpp` / `header.h`: Core application logic, steganographic algorithms, and CLI parsing.

 `test.bat`: Windows batch file to quickly test encoding and decoding workflows.

 `main.exe`: Pre-compiled executable for Windows environments.

 `.vscode/`: Configuration files for Visual Studio Code.



## Prerequisites

 A Windows environment (the provided build/test files assume Windows paths and produce a `.exe`).

 A C++17-capable compiler. The project is built and tested with the [MSYS2](https://www.msys2.org/) `ucrt64` toolchain (`g++`), installed by default to `C:\msys64`.

 The following dependencies are vendored under `include/` and don't need to be installed separately:
  - [`stb`](https://github.com/nothings/stb) (image read/write, header-only)
  - [`CLI11`](https://github.com/CLIUtils/CLI11) (command line parsing, header-only)
  - [`libsndfile`](https://github.com/libsndfile/libsndfile) (WAV read/write, headers + import lib only)

 `libsndfile`'s runtime `sndfile.dll` is **not** checked into this repository (only its headers and import library are, under `include/libsndfile/`). Download a matching `libsndfile` Windows release and place `sndfile.dll` in the project root before building/running, or the executable won't start.



## Compilation

Open the project folder in VS Code with the MSYS2 `ucrt64` `bin` directory on your `PATH`, then run the default build task (`Ctrl+Shift+B`). This uses the command defined in `.vscode/tasks.json`:

```
g++ -fdiagnostics-color=always -g src/*.cpp -Iinclude/stb -Iinclude/CLI11 -Iinclude/libsndfile/include sndfile.dll -static -o main.exe
```

Equivalently, from a terminal with the same toolchain on `PATH`, run the same command from the project root. This produces `main.exe` in the project root.



## Usage

```
main.exe [--verbose] (--image | --audio) encode --into <file-or-folder> --from <file>
main.exe [--verbose] (--image | --audio) decode --modified <file-or-folder> --original <file-or-folder>
```

 `--image` / `-m`: treat `--into` as a PNG file, or a folder of PNG files.
 `--audio` / `-a`: treat `--into` as a WAV file.
 `--verbose` / `-v`: print progress output instead of running silently.

Encoding always hides the file given by `--from` inside a *copy* of `--into`; the original is left untouched. Decoding needs both the modified copy (`--modified`) and the original (`--original` / `--into`) to diff against and recover the hidden data.

Output naming:
 Single image encode always writes to `output.png` in the current directory.
 Folder image encode writes to a sibling folder named `<into-folder>M`, with an `M` inserted before each file's extension.
 Audio encode writes next to the input, with an `M` inserted before the extension (e.g. `sound.wav` → `soundM.wav`).

### Examples

Hide a text file inside a PNG:

```
main.exe --verbose --image encode --into original.png --from secret.txt
```

Recover it from the encoded copy (`output.png`) by diffing against the original:

```
main.exe --verbose --image decode --modified output.png --original original.png
```

Hide a file inside every PNG in a folder:

```
main.exe --verbose --image encode --into imagesOriginal --from secret.txt
```

Hide a file inside a WAV file:

```
main.exe --verbose --audio encode --into sound.wav --from secret.txt
```

See `test.bat` for full worked examples of all three modes, including verifying the round trip with SHA-256 hashes.


