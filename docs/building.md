# This is the Documentation for building P99. 

## 1. Clone the repository  
Depending on what you are trying to do you might have to switch to the "Development" Branch which includes the newest experimental features (so may be unstable)  
Run `git clone https://github.com/mrHunor/P99` (for the stable branch) or `git clone -b Development https://github.com/mrHunor/p99` (for the Development branch)  

## 2. Verfiy you have Cmake (Version>= 3.2.0) and MSYS g++ (Version>=15.2.0) installed 
Run  `cmake --version` in your terminal  
Run  `g++ --version` in your terminal  

## 3. Build! 
Run `cmake -S . -B build` in the root of the Directory to generate a build folder  
Run `cmake --build build` in the root of the Directory to generate the executable (placed in root/build/)

### For any Questions please refer to *https://github.com/MrHunor/P99/issues*