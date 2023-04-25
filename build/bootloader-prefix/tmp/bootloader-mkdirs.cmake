# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/AM/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix/tmp"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix/src"
  "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/AM/Documents/Vscode/IoT/tcp_server/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
