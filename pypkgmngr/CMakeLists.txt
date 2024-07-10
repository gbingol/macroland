cmake_minimum_required(VERSION 3.27.2)
set(CMAKE_CXX_STANDARD 20)

if(NOT WIN32)
	message(FATAL_ERROR "Works only under Win32")
endif()


project(pypkgmngr)


#wxWidgets directories
include_directories(C:/_libs/wxWidgets/include)
include_directories(C:/_libs/wxWidgets/lib/mswu)
include_directories(${CMAKE_SOURCE_DIR})

#Python directory
include_directories(${PYHOME}/include)


file(GLOB pypkgmngrfiles
     "*.h" 
	 "*.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${pypkgmngrfiles})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DPI_AWARE "PerMonitor")

target_compile_definitions(${PROJECT_NAME} PRIVATE 
_WINDOWS _USRDLL _UNICODE __WXMSW__ wxUSE_NATIVE_DATAVIEWCTRL
		wxUSE_MENUBAR WXUSINGDLL _CRT_SECURE_NO_WARNINGS _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

target_link_directories(${PROJECT_NAME} PRIVATE C:/_libs/wxWidgets/lib)
target_link_directories(${PROJECT_NAME} PRIVATE ${PYHOME}/libs)


target_link_libraries(${PROJECT_NAME} 
	python3 ${PYLIB} wxmsw32u )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy 
	$<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_SOURCE_DIR}/64Bit/${PROJECT_NAME}.exe)