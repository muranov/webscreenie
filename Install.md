#Instruction for building / installing webscreenie

# Prerequisite #

## For qmake ##

  * Qt 4.5+

## For cmake ##

  * Qt 4.5+
  * cmake 2.6+

# Getting the source #

The source is available through svn (subversion) on google code. The project url is:
https://webscreenie.googlecode.com/svn/trunk

# Building with qmake #

qmake is the build utility for Qt. A project file (.pro) is provided with webscreenie so building with qmake is very straightforward:

  1. "cd" to the root of the source
    * cd /home/myuser/src/webscreenie
  1. Create a temporary build directory
    * mkdir build
    * cd build
  1. Create the MakeFile with qmake
    * qmake ../webscreenie.pro
  1. Build the project with make
    * make
  1. That's all. You should have a compiled binary named "webscreenie" in your build directory

Run webscreenie --help for more information about the command line options

# Building with cmake #

Alternately, a cmake file is also provided if you prefer to build the project using cmake:

  1. "cd to the root of the source
    * cd /home/myuser/src/webscreenie
  1. Create a temporary build directory
    * mkdir build
    * cd build
  1. Create the Makefile with cmake
    * cmake ..
  1. Build the project with make
    * make
  1. That's all. You should have a compiled binary named "webscreenie" in your build directory

Run webscreenie --help for more information about the command line options

---
Note: Qt must be compiled with QtWebkit and QtScript enabled