# P99 - File to Media File Steganography © 2026 MrHunor, GPLv3

## Features:
- File to Single Image/Image Folder encoding/decoding
- File to Single Wav/Wav Folder encoding/decoding 
- Capactiy Checking of a File

## Building:
***This is an excerpt from docs/building.md***  
### 1. Clone the repository  
Depending on what you are trying to do you might have to switch to the "Development" Branch which includes the newest experimental features (so may be unstable)  
Run `git clone https://github.com/mrHunor/P99` (for the stable branch) or `git clone -b Development https://github.com/mrHunor/p99` (for the Development branch)  

### 2. Verfiy you have Cmake (Version>= 3.2.0) and MSYS g++ (Version>=15.2.0) installed 
Run  `cmake --version` in your terminal  
Run  `g++ --version` in your terminal  

### 3. Build! 
Run `cmake -S . -B build` in the root of the Directory to generate a build folder  
Run `cmake --build build` in the root of the Directory to generate the executable (placed in root/build/)

## Usage:
`main.exe [OPTIONS] [SUBCOMMANDS]`


`OPTIONS:`
` -h,     --help              Print this help message and exit`  
`          --help-all          Expand and show all subcommand options`  
`  -v,     --verbose [0]       Enable verbose output`  
`[Option Group: Media Into Type]`  

 ` [Exactly 1 of the following options are required]`  


`OPTIONS:`  
`  -a,     --audio             Into File or all Files in the Folder is/are Audio File`  
`  -m,     --image             Into File or all Files in the Folder is/are image`  

`SUBCOMMANDS:`  
`  encode                      Encode a File into a image or folder containing images`  
`  decode                      Decode a File from a image or folder containing images`  
`  check                       Check capacity of a Media File (currently only supportes images)`  
