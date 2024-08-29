project(macrolandapp)


include_directories(C:/_libs/boost/)

#Python directory
include_directories(${PYHOME}/include)


file(GLOB macrolandfiles
     "*.h" "*.cpp"
	  "cmdwnd/*.h" "cmdwnd/*.cpp"
	  "grid/*.h" "grid/*.cpp"
	 "lua/*.h" "lua/*.cpp"
	 "mainfrm/*.h" "mainfrm/*.cpp"
	 "python/*.h" "python/*.cpp"
	 "util/*.h" "util/*.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${macrolandfiles})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
_WINDOWS _USRDLL _UNICODE __WXMSW__ WXUSINGDLL _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)


find_package(wxWidgets REQUIRED core base aui net xml stc)
find_package(Lua REQUIRED)

include(${wxWidgets_USE_FILE})
target_include_directories(${PROJECT_NAME} PRIVATE 
					${wxWidgets_INCLUDE_DIRS} 
					${LUA_INCLUDE_DIR})

target_link_directories(${PROJECT_NAME} PRIVATE 
						${wxWidgets_LIBRARY_DIRS} 
						${PYHOME}/libs 
						C:/_libs/boost/lib64-msvc-14.3) #libboost_json-vc143-mt-x64-1_81.lib

target_link_libraries(${PROJECT_NAME}  
	python3  
	${wxWidgets_LIBRARIES} 
	${PYLIB} 
	${LUA_LIBRARIES})

