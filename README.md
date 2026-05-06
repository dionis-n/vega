# Vega
A cross-platform scheduling application for MIREA students, built with WebAssembly for browser access.

<p align="center">
  <img width="350" alt="settings" src="https://github.com/user-attachments/assets/7603befc-2e4e-45ae-880e-ff945524bec2" />
  <img width="350" alt="schedule" src="https://github.com/user-attachments/assets/29fb71e5-9477-4f9a-bba3-bc876ea97c01" />
</p>

## Table of Contents
* [Technologies Used](#technologies-used)
* [Features](#features)
* [Build](#build)
* [Usage](#usage)
* [License](#license)


## Technologies Used
- Qt 6.11.0
- C++17
- CMake 3.16
- WebAssembly (Emscripten)
- OpenXLSX


## Features
- Cross-platform via browser
- Offline work (PWA support)
- Dark and light themes
- Horizontal and vertical orientation
- Support for odd/even weeks and specific week schedules
- Multiple group support
- Lesson type display (lecture/practice)


## Build
Fork the repository and import to Qt Creator with WebAssembly kit.

### WebAssembly
1. Install Emscripten SDK
2. Open project in Qt Creator with WebAssembly kit
3. Build and run

## Usage
Open `Vega.html` in a browser or deploy to any web server. 
Load your `.xlsx` schedule file in the Settings tab.


## License
LGPL
