project(macrolandapp)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/64Bit)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/64Bit)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/64Bit)


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


include_directories(C:/_libs/boost/)
include_directories(C:/_libs/lua/include)
include_directories(${CMAKE_SOURCE_DIR}/wxgrid)
include_directories(${CMAKE_SOURCE_DIR}/jsonparser)

#Python directory
include_directories(${PYHOME}/include)


file(GLOB macrolandfiles
     "macroland/*.h" "macroland/*.cpp"
	  "macroland/cmdwnd/*.h" "macroland/cmdwnd/*.cpp"
	 "macroland/lua/*.h" "macroland/lua/*.cpp"
	 "macroland/mainfrm/*.h" "macroland/mainfrm/*.cpp"
	 "macroland/python/*.h" "macroland/python/*.cpp"
	 "macroland/util/*.h" "macroland/util/*.cpp"
	 ${CMAKE_SOURCE_DIR}/jsonparser/json.cpp #no need to make single file a library
)

add_executable(${PROJECT_NAME} WIN32 ${macrolandfiles})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
_WINDOWS _USRDLL _UNICODE __WXMSW__ wxUSE_NATIVE_DATAVIEWCTRL
		wxUSE_MENUBAR WXUSINGDLL _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)


find_package(wxWidgets REQUIRED core base aui net xml stc)

include(${wxWidgets_USE_FILE})
target_include_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_INCLUDE_DIRS})

target_link_directories(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARY_DIRS} )
target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/boost/lib64-msvc-14.3)
target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/lua/)
target_link_directories(${PROJECT_NAME} PRIVATE ${PYHOME}/libs)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/wxgrid/libs)

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} )
target_link_libraries(${PROJECT_NAME}  python3 ${PYLIB} grid$<$<CONFIG:Debug>:_d> lua54)

