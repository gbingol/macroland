cmake_minimum_required(VERSION 3.27.2)
set(CMAKE_CXX_STANDARD 20)


project(macrolandapp)


#wxWidgets directories
include_directories(C:/_libs/wxWidgets/include)
include_directories( $<$<CONFIG:Release>:C:/_libs/wxWidgets/lib/mswu>)
include_directories( $<$<CONFIG:Debug>:C:/_libs/wxWidgets/lib/mswud>)

include_directories(C:/_libs/boost/)
include_directories(C:/_libs/lua/include)
include_directories(${CMAKE_SOURCE_DIR})

#Python directory
include_directories(${PYHOME}/include)


file(GLOB macrolandfiles
     "*.h" 
	 "*.cpp"
	 "icell/*.h" 
	 "icell/*.cpp"
	 "lua/*.h" 
	 "lua/*.cpp"
	 "mainfrm/*.h" 
	 "mainfrm/*.cpp"
	 "python/*.h" 
	 "python/*.cpp"
	 "cmdwnd/*.h" 
	 "cmdwnd/*.cpp"
	 "util/*.h" 
	 "util/*.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${macrolandfiles})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
$<$<CONFIG:Debug>:_DEBUG>
$<$<CONFIG:Release>:NDEBUG>
_WINDOWS _USRDLL _UNICODE __WXMSW__ wxUSE_NATIVE_DATAVIEWCTRL
		wxUSE_MENUBAR WXUSINGDLL _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)


target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/wxWidgets/lib)
target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/boost/lib64-msvc-14.3)
target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/lua/)
target_link_directories(${PROJECT_NAME} PRIVATE ${PYHOME}/libs)


target_link_libraries(${PROJECT_NAME}  
	$<$<CONFIG:Release>:wxmsw32u>
	$<$<CONFIG:Debug>:wxmsw32ud>
	python3 ${PYLIB}
	scisuit_grid
	lua54)
