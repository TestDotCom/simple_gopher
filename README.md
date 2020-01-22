# GOPHER SERVER

## Project 2018-2019
Programmazione di Sistema - Sapienza

## Requested funtions

- portable between Windows and Linux
- default port 7070
- handle SIGHUP (Linux) or console event (Windows): read conf file
- multi-thread/process support: for every new connection spawn a child/thread
- main process waits on select()
- exclusive read/write access to files
- send memory mapped files from another thread
- find file's type calling popen (linux) or via extension (Windows)
- every file sent is recorded on a log file, handled by another process
    - file name
    - file size
    - client's IP address and port number
- run as daemon under linux

### Supported file type

- 0 : directory
- 1 : file
- 3 : error
- 9 : binary (application)
- I : image
- s : audio

## how-to build (under linux)

### pre-requisites

- mingw64 environment (windows build)
- clang
- make & cmake
- google sanitizer libs (debug build)

### linux

- cd build_nix
- cmake ..
- make

### windows

Instanll mingw64, then:

- cd build_win
- mingw64-cmake ..
- mingw64-make
