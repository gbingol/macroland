cmake_minimum_required(VERSION 3.27.2)
set(CMAKE_CXX_STANDARD 20)

if(NOT WIN32)
	message(FATAL_ERROR "Works only under Win32")
endif()

project(macrolandframework)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/64Bit)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/64Bit)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/64Bit)


cmake_path(GET CMAKE_SOURCE_DIR PARENT_PATH PROJ_PARENT_DIR)
SET(GRID_PATH ${PROJ_PARENT_DIR}/wxgrid)


#set(USER_HOME_DIRECTORY $ENV{USERPROFILE})
#message(STATUS "Home Dir: ${USER_HOME_DIRECTORY}")

set(CMAKE_DEBUG_POSTFIX "_d")


set(VARPY Python310)
#set(VARPY Python311)
#set(VARPY Python312)

if(${VARPY} STREQUAL "Python310")
	set(PYLIB "python310")
	set(PYHOME "C:/Python/Python3106/")

elseif(${VARPY} STREQUAL "Python311")
	set(PYLIB "python311")
	set(PYHOME "C:/Python/Python311/")

elseif(${VARPY} STREQUAL "Python312")
	set(PYLIB "python312")
	set(PYHOME "C:/Python/Python312/")
endif()

include(C:/vcpkg/scripts/buildsystems/vcpkg.cmake)


add_subdirectory(macroland)

