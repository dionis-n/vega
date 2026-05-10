# Install script for directory: C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/emsdk/upstream/emscripten/cache/sysroot")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/lMingw64andSFML/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX/headers" TYPE FILE FILES "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/OpenXLSX-Exports.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX/headers" TYPE FILE FILES
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/IZipArchive.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCell.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCellIterator.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCellRange.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCellReference.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCellValue.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLColor.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLColumn.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLCommandQuery.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLConstants.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLContentTypes.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLDateTime.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLDocument.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLException.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLFormula.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLIterator.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLProperties.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLRelationships.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLRow.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLRowData.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLSharedStrings.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLSheet.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLWorkbook.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLXmlData.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLXmlFile.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLXmlParser.hpp"
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/headers/XLZipArchive.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX" TYPE FILE FILES "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/OpenXLSX.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/libOpenXLSXd.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES
    "C:/Users/ADMIN/Documents/vega/src/Backend/OpenXLSX/OpenXLSXConfig.cmake"
    "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/OpenXLSX/OpenXLSXConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake"
         "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES "C:/Users/ADMIN/Documents/vega/src/build/WebAssembly_Qt_6_11_0_single_threaded-u041eu0442u043bu0430u0434u043au0430/Backend/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets-debug.cmake")
  endif()
endif()

