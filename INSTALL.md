[![Build Status](https://travis-ci.org/rdaly525/coreir.svg?branch=master)](https://travis-ci.org/rdaly525/coreir)

## Tested Compatable compilers:  
  gcc 4.9  
  Apple LLVM version 8.0.0 (clang-800.0.42.1)  

## How to Install from source
### If you are using osx:  
Add `export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:<path_to_coreir>/lib` to your `~/.bashrc` or `~/.profile`

### If you are using linux:  
Add `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path_to_coreir>/lib` to your `~/.bashrc` or `~/.profile` 

### To Install:
    
    `cd <path_to_coreir>`
    `make install`

### To verify coreir build
    
    `make test`

### Install standalone binary

    `make coreir`

## How to Install Python Bindings

### To Install: 

    `cd <path_to_coreir>`
    `make py`
  
### To verify python bindings
    
    `make pytest`

