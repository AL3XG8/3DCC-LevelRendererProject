@echo off

:: This script needs three arguments to do its job 
:: Argument 1 - the relative path from this script to the folder containing the cmakelists.txt 
:: Argument 2 - the relative path from this script to the folder where Gateware.h needs to be
:: Argument 3 - the name of the cmake project (can be found in the cmakelists.txt) 

::clone Gateware.h to expected location 
echo :
echo :
echo //CLONING GATEWARE//
set "previousDirectory=%cd%" 
mkdir "%~2"
cd "%~2"
git clone https://gitlab.com/gateware-development/gateware.git --depth=1 --single-branch --quiet
cd gateware
copy "./Gateware.h" "../"
cd %previousDirectory%

::make the project
echo :
echo :
echo //MAKING PROJECT//
cmake -S "./%~1" -B "./%~1/build"

::build the project 
echo :
echo :
echo //BUILDING PROJECT IN DEBUG//
cmake --build "./%~1/build" --target "%~3" --config "Debug"

echo :
echo :
echo //BUILDING PROJECT IN RELEASE//
cmake --build "./%~1/build" --target "%~3" --config "Release"


::Delete gateware and the build 
echo :
echo :
echo //CLEANING UP//
if exist "./%~1/build" rmdir /s /q "./%~1/build"
if exist "./%~2" rmdir /s /q "./%~2"
echo - deleted build folder and gateware clone -

exit /B %ERRORLEVEL%