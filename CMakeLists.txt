cmake_minimum_required (VERSION 2.8.8)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "-std=c++11")
project(ibus-engine-myengine) #project name

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY} )  #root top

set(CMAKE_INSTALL_PREFIX "/usr/libexec/") #install prefix  cache

add_subdirectory(src)

install(FILES icons/ibus-myengine.svg DESTINATION /usr/share/ibus-myengine/icons/)
