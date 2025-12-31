@echo off
set BUILD_DIR=build

if exist %BUILD_DIR% (
    echo Cleaning build directory...
    rmdir /s /q %BUILD_DIR%
)

mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo Configuring...
cmake ..
if %errorlevel% neq 0 exit /b %errorlevel%

echo Building Debug...
cmake --build . --config Debug
if %errorlevel% neq 0 exit /b %errorlevel%

echo Building Release...
cmake --build . --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo Packaging...
cpack -C "Debug;Release"
if %errorlevel% neq 0 exit /b %errorlevel%

echo Done!
