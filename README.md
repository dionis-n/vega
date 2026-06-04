# Vega
A cross-platform scheduling application for MIREA students, built with WebAssembly for browser access.

<p align="center">
  <img width="427" height="758" alt="image" src="https://github.com/user-attachments/assets/d6bc7ae1-2f0c-4201-a539-bc079979ad78" />
  <img width="437" height="759" alt="image" src="https://github.com/user-attachments/assets/9e7dd607-48ee-4060-8581-4970efdeb3a7" />
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
Vega is built as a WebAssembly app. The same code also compiles natively as `VegaTests`, used only to run the parser test suite.

### Prerequisites
- Qt 6.11.0 with the *WebAssembly (single-threaded)* target
- A matching host Qt 6.11.0 desktop build (provides `moc`/`rcc`/`uic`): `gcc_64` on Linux, `mingw_64` on Windows
- Emscripten SDK 4.0.7 (`emsdk`)
- CMake 3.16+ and Ninja

### WebAssembly (the app)
With Qt Creator: activate emsdk (`emsdk activate 4.0.7`), open `src/CMakeLists.txt` with the *WebAssembly Qt 6.11.0 single-threaded* kit, and build the `Vega` target.

From the command line (same as CI):
```bash
# activate emsdk first: source emsdk_env.sh  (Windows: emsdk_env.ps1)
cmake -S src -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$QT_WASM/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DQT_HOST_PATH="$QT_HOST" -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Vega
```
`$QT_WASM` is your `…/6.11.0/wasm_singlethread`, `$QT_HOST` is `…/6.11.0/gcc_64` (or `mingw_64`).

The build produces `Vega.html`, `Vega.js`, `Vega.wasm` plus the PWA assets. WebAssembly can't load from `file://`, so serve it over HTTP:
```bash
python -m http.server -d build 8000   # → http://localhost:8000/Vega.html
```

### Tests (native)
```bash
cmake -S src -B build-native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-native --target VegaTests
ctest --test-dir build-native --output-on-failure
```
Run against a real schedule:
```bash
./build-native/VegaTests --schedule=src/Tests/data/sample_schedule.xlsx
```


## Usage
Open `Vega.html` in a browser or deploy to any web server. 
Load your `.xlsx` schedule file in the Settings tab.


## License
LGPL
