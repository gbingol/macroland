cmake_minimum_required(VERSION 3.27.2)
set(CMAKE_CXX_STANDARD 20)


project(scisuit_grid)


#wxWidgets directories
include_directories(C:/_libs/wxWidgets/include)
include_directories( $<$<CONFIG:Release>:C:/_libs/wxWidgets/lib/mswu>)
include_directories( $<$<CONFIG:Debug>:C:/_libs/wxWidgets/lib/mswud>)
include_directories(C:/_libs/boost/)


file(GLOB cppfiles
     "*.h"
     "*.cpp"
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(${PROJECT_NAME} SHARED ${cppfiles})

target_compile_definitions(${PROJECT_NAME} PRIVATE 
$<$<CONFIG:Debug>:_DEBUG>
$<$<CONFIG:Release>:NDEBUG>
GUI_EXPORTS _WINDOWS _USRDLL _UNICODE __WXMSW__ wxUSE_NATIVE_DATAVIEWCTRL
		wxUSE_MENUBAR WXUSINGDLL _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/wxWidgets/lib)
target_link_directories(${PROJECT_NAME} PRIVATE $ENV{VSPROJ}/sci_libs/libs)
target_link_directories(${PROJECT_NAME} PRIVATE ${PYHOME}/libs)


target_link_libraries(${PROJECT_NAME}  
$<$<CONFIG:Release>:wxmsw32u>
$<$<CONFIG:Debug>:wxmsw32ud>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy 
	$<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_SOURCE_DIR}/64Bit/${PROJECT_NAME}.dll)