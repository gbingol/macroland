cmake_minimum_required(VERSION 3.27.2)
set(CMAKE_CXX_STANDARD 20)


project(scisuit_grid)


file(GLOB cppfiles "*.h" "*.cpp" )

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(${PROJECT_NAME} SHARED ${cppfiles})

target_compile_definitions(${PROJECT_NAME} PRIVATE GRIDEXPORTS 
_WINDOWS _USRDLL _UNICODE __WXMSW__ WXUSINGDLL 
_CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

find_package(wxWidgets REQUIRED core base aui net xml)

include(${wxWidgets_USE_FILE})
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARY_DIRS} )
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} )