project(macrolandapp)


include_directories(C:/_libs/boost/)
include_directories(C:/_libs/lua/include)
include_directories(${GRID_PATH})

#Python directory
include_directories(${PYHOME}/include)


file(GLOB macrolandfiles
     "*.h" "*.cpp"
	 "icell/*.h" "icell/*.cpp"
	 "lua/*.h" "lua/*.cpp"
	 "mainfrm/*.h" "mainfrm/*.cpp"
	 "python/*.h" "python/*.cpp"
	 "cmdwnd/*.h" "cmdwnd/*.cpp"
	 "util/*.h" "util/*.cpp"
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
target_link_directories(${PROJECT_NAME} PRIVATE ${GRID_PATH}/libs)

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} )
target_link_libraries(${PROJECT_NAME}  python3 ${PYLIB} grid lua54)


#copy grid.dll to 64Bit Dir
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy 
	${GRID_PATH}/libs/grid.dll ${CMAKE_SOURCE_DIR}/64Bit/grid.dll)

