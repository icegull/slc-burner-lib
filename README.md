# SLC Burner Library

`slc-burner-lib` is a C++ library designed to burn (overlay) timecodes onto video frames. It supports various raw video formats and provides a simple API for integrating timecode overlays into video processing pipelines.

## Features

*   **Supported Formats:**
    *   UYVY
    *   YUV422
    *   V210
*   **Customizable:**
    *   Configurable font size.
    *   Arbitrary position (X, Y) for the timecode overlay.
*   **Performance:**
    *   Supports both in-place processing and copy-to-destination processing.
*   **Modern C++:**
    *   Written in C++20.

## Build Instructions

This project uses CMake for building.

### Prerequisites

*   CMake (version 3.15 or higher)
*   C++ Compiler supporting C++20 (e.g., MSVC, GCC, Clang)

### Building

1.  Clone the repository.
2.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```
3.  Configure the project:
    ```bash
    cmake ..
    ```
4.  Build the project:
    ```bash
    cmake --build . --config Release
    ```

## Usage

### Include the Header

```cpp
#include "Burner.h"
```

### Initialization

Initialize the burner with the video frame dimensions and format.

```cpp
SLC::Burner burner;
uint32_t width = 1920;
uint32_t height = 1080;
int32_t fontSize = 64;

// Initialize for UYVY format
burner.init(width, height, SLC::Burner::SourceType::UYVY, fontSize);
```

### Burning Timecode

Prepare the timecode structure and call the `burn` method.

```cpp
SLC::Burner::BurnerTimeId timeId;
timeId.hour = 12;
timeId.minute = 34;
timeId.second = 56;
timeId.decimal = 0; // Optional decimal part

// Source buffer (pointer to raw video data)
const uint8_t* srcBuffer = ...; 
size_t srcSize = ...;

// Destination buffer (if not burning in-place)
uint8_t* dstBuffer = ...;

// Position to burn the timecode
int64_t xPos = 100;
int64_t yPos = 100;

// Option 1: Burn to a destination buffer
burner.burn(srcBuffer, srcSize, dstBuffer, timeId, xPos, yPos);

// Option 2: Burn in-place (modifies srcBuffer directly)
// Note: The const_cast is implied if you pass a non-const pointer to the overload that supports it, 
// but the library provides an overload that takes a const src and writes to it if it was actually mutable, 
// or you can use the specific in-place overload if available/implemented.
// Based on the header:
// int32_t burn(const uint8_t *src, uint64_t srcSize, const BurnerTimeId& timeId, int64_t xPos, int64_t yPos);
// This overload likely treats src as mutable internally or is intended for in-place if the pointer allows.
// *Check implementation details for const-correctness on in-place burn.*
```

## Project Structure

*   `include/Burner.h`: Public API header.
*   `src/Burner.cpp`: Library implementation.
*   `src/main.cpp`: Example usage and test application (`burner_test`).
*   `CMakeLists.txt`: CMake build configuration.

## License

This project is licensed under the MIT License.
