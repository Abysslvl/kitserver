# Introduction

Kitserver 12 is a companion program for Pro Evolution Soccer 2012.

# How to build

Visual Studio 2022 Preview Edition is used for this project but you
should be able to use other versions too (with cmake integration supported).

Run the command-line in the project folder, then:

    git submodules update --init --recursive

Open the project folder with Visual Studio and manage project settings, then:
    
    - set x86-release/x86-debug build type.
    - set the CMAKE_TOOLCHAIN_FILE to use vcpkg's.
    - Generate, compile & install kitserver.

Dont forget include "for_releases" content and "doc" folder in your releases.