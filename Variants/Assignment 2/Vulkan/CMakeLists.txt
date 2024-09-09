cmake_minimum_required(VERSION 3.10)

project(Assignment_2_Vulkan)

set(VERTEX_SHADERS 
	# add vertex shader (.hlsl) files here
	Shaders/VertexShader.hlsl
)

set(PIXEL_SHADERS 
	# add pixel shader (.hlsl) files here
	Shaders/FragmentShader.hlsl
)

# Add any new C/C++ source code here
set(SOURCE_CODE
	# Header & CPP files go here
	main.cpp
	renderer.h
	FileIntoString.h
)

if(WIN32)
	# by default CMake selects "ALL_BUILD" as the startup project
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Assignment_2_Vulkan)

	#this will tell Visual Studio to put the shaders under their own group in the solution explorer 
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VERTEX_SHADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PIXEL_SHADERS})
endif()

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (WIN32)
	# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")

	add_executable (Assignment_2_Vulkan 
	${SOURCE_CODE}
	${VERTEX_SHADERS}
	${PIXEL_SHADERS}
	)

	target_include_directories(Assignment_2_Vulkan PUBLIC $ENV{VULKAN_SDK}/Include/)
	target_link_directories(Assignment_2_Vulkan PUBLIC $ENV{VULKAN_SDK}/Lib/)
endif(WIN32)

if(UNIX AND NOT APPLE)
	# libshaderc_combined.a is required for runtime shader compiling
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lshaderc_combined")
    find_package(X11)
	find_package(Vulkan REQUIRED)
    link_libraries(${X11_LIBRARIES})
    include_directories(${X11_INCLUDE_DIR})
    include_directories(${Vulkan_INCLUDE_DIR}) 
	#link_directories(${Vulkan_LIBRARY}) this is currently not working
	link_libraries(${Vulkan_LIBRARIES})
	# the path is (properly)hardcoded because "${Vulkan_LIBRARY}" currently does not 
	# return a proper path on MacOS (it has the .dynlib appended)
    link_libraries(/usr/lib/x86_64-linux-gnu/libshaderc_combined.a)

	add_executable (Assignment_2_Vulkan 
	${SOURCE_CODE}
	${VERTEX_SHADERS}
	${PIXEL_SHADERS}
	)
endif(UNIX AND NOT APPLE)

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fmodules -fcxx-modules")
	set(Architecture ${CMAKE_OSX_ARCHITECTURES})
	find_package(Vulkan REQUIRED)
	include_directories(${Vulkan_INCLUDE_DIR}) 
	#link_directories(${Vulkan_LIBRARY}) this is currently not working
	link_libraries(${Vulkan_LIBRARIES})
	# libshaderc_combined.a is required for runtime shader compiling
	# the path is (properly)hardcoded because "${Vulkan_LIBRARY}" currently does not 
	# return a proper path on MacOS (it has the .dynlib appended)
	link_libraries(/usr/local/lib/libshaderc_combined.a)
	add_executable (Assignment_2_Vulkan main.mm)
endif(APPLE)

#exclude stuff from build
set_source_files_properties(${VERTEX_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "none")
set_source_files_properties(${PIXEL_SHADERS} PROPERTIES VS_TOOL_OVERRIDE "none")