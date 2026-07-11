# This is the Documentation for all functions in the **src/core** directory. 

## defs.h 
### `class stateClass` 
-> `void stateClass.out(const std::string &*output*, int *importance*, std::source_location *location* = std::source_location::current())`  
***General console output function***  
`void` = Returns nothing  
`const std::string &*output*` = Whatever you want to output to console  
`int *importance*` = The Level of importance your output has (1-infinity, where 1 is the highest and 4 is the lowest -> -v = 1, -vvvv = 4)  
`std::source_location *location*` = Location of the function calling .out to provide a more presise logging (should be left empty because default argument is provided)  

## utils.cpp  
### `void InvalidInputMessage(const std::string &*details*, std::source_location *location*)` 
***Crash message and exit function***  
`void` = Returns nothing  
`const std::string &*details*` = Whatever details you can provide to the crash  
`std::source_location *location*` = Location of the function calling the function  to provide a more presise logging (should be left empty because default argument is provided in the respective header)   

### `bool createFolder(const std::string &name, stateClass &state)`

