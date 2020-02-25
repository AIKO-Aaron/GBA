# GBA Emulator

## About

    This is a very work in progress GBA emulator
    There might be a game that boots & emulates up to a point...

## Build
### Dependencies
    Download and install the dependencies (SDL2 & SDL2_ttf)
    To do so just run the downloader.sh on MacOS or Linux (should work on both)
    For Windows download and place the libraries into the dependencies folder. Open & Configure the GBA.sln file with Visual Studio

    Download a gba bios & a game
    Place the bios as 'bios.gba' and the game as 'emerald.gba'
    into the GBA folder

### Linux & MacOS:
    Adjust the makefile to only copy emerald.gba and bios.gba and
    maybe the Raleway font if you want to use the debugger.
 
    Create a bin directory in this folder

    To build you can either run ```make```
    and go to the bin folder and run
    ```LD_LIBRARY_PATH=. ./gba```
    or just go into the GBA folder and run ./easy_run.sh to build & run for the first time (or if you want to recompile the project). Else you can just run run.sh
    
### Windows:
    Open the GBA Solution file and press F5


## TODO:
- Sprites
- Tests