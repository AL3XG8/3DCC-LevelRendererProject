::This script just makes a project using cmake. 
::Give it the relative path from this script to the folder containing the cmakelists.txt 

@echo off
::make the project
echo :
echo :
echo //MAKING PROJECT//
cmake -S "./%~1" -B "./%~1/build"

::Delete the build 
echo :
echo :
echo //CLEANING UP//
if exist "./%~1/build" rmdir /s /q "./%~1/build"
echo - deleted build folder -