cmake_minimum_required(VERSION 3.7.1)

set(Example_Name "UIRenderingConcepts")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # 
set(EXAMPLE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/../../..")
set(EXTERNAL_DIR "${EXAMPLE_SOURCE_DIR}/external")

# Include Qt basic functions
include(QtCommon)
# Set PROJECT_VERSION_PATCH and PROJECT_VERSION_TWEAK to 0 if not present, needed by add_project_meta
fix_project_version()

# Specify a suitable project name
project(${Example_Name}  VERSION 1.0.0)
add_project_meta(META_FILES_TO_INCLUDE)

find_package(Qt5Widgets REQUIRED)

message(STATUS "Attempting auto locate Vulkan using CMake......")
# Find Vulkan Path using CMake's Vulkan Module
# This will return Boolean 'Vulkan_FOUND' indicating the status of find as success(ON) or fail(OFF).
# Include directory path - 'Vulkan_INCLUDE_DIRS' and 'Vulkan_LIBRARY' with required libraries.
find_package(Vulkan)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
	set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
	STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()
		 
if(NOT Vulkan_FOUND)
	# CMake may fail to locate the libraries but could be able to 
	# provide some path in Vulkan SDK include directory variable
	# 'Vulkan_INCLUDE_DIRS', try to extract path from this.
	message(STATUS "Failed to locate Vulkan SDK, retrying again...")
	if(EXISTS "${VULKAN_PATH}")
		message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
	else()
		message(STATUS "Attempting to locate Vulkan SDK using manual path......")
		set(VULKAN_SDK "C:/VulkanSDK")
		set(VULKAN_VERSION "1.0.46.0")
		set(VULKAN_PATH "${VULKAN_SDK}/${VULKAN_VERSION}")
		message(STATUS "Using manual specified path: ${VULKAN_PATH}")

		# Check if manual set path exists
		if(NOT EXISTS "${VULKAN_PATH}")
			message("Error: Unable to locate this Vulkan SDK path VULKAN_PATH: ${VULKAN_PATH}, please specify correct path.")
		   return()
		endif()
		return()
	endif()
endif()

# Add any required preprocessor definitions here
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Include Vulkan header files from Vulkan SDK
	include_directories(AFTER ${VULKAN_PATH}/Include)

	# Link directory for vulkan-1
	link_directories(${VULKAN_PATH}/Bin)
	link_directories(${VULKAN_PATH}/Lib)
#	link_directories(${EXAMPLE_SOURCE_DIR}/libs/assimp)
endif()

# GLM SETUP - Mathematic libraries for 3D transformation
set(GLMINCLUDES "${EXTERNAL_DIR}")
get_filename_component(GLMINC_PREFIX "${GLMINCLUDES}" ABSOLUTE)
if(NOT EXISTS ${GLMINC_PREFIX})
    message(FATAL_ERROR "Necessary glm headers do not exist: " ${GLMINC_PREFIX})
endif()
include_directories( ${GLMINC_PREFIX} )

#set(ASSIMP_INCLUDES "${EXTERNAL_DIR}/assimp")
#include_directories(ASSIMP_INCLUDES)

# Define directories and the contained folder and files inside.
if(WIN32)
    source_group("include" REGULAR_EXPRESSION "include/*.h")
    source_group("source" REGULAR_EXPRESSION "source/*.cpp")
#	find_library(ASSIMP_LIBRARIES NAMES assimp libassimp.dll.a PATHS ${EXAMPLE_SOURCE_DIR}/libs/assimp)
endif(WIN32)

# Gather list of header and source files for compilation
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.*)
file(GLOB_RECURSE VULKAN_CORE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../engine/vulkan/core/*.*)
file(GLOB_RECURSE VULKAN_IMPLEMENTATION_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../engine/vulkan/implementation/*.*)
file(GLOB_RECURSE SCENE_GRAPH_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../common/SceneGraph/*.*)

# Build project, give it a name and includes list of file to be compiled
add_executable(${Example_Name} ${SOURCE_FILES} ${VULKAN_CORE_FILES} ${VULKAN_IMPLEMENTATION_FILES} ${RESOURCE_FILES} ${SCENE_GRAPH_FILES})

# Link the debug and release libraries to the project
target_link_libraries( ${Example_Name} vulkan-1)

# Define project properties
set_property(TARGET ${Example_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Example_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Example_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Example_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Example_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/binaries)

# Define C++ version to be used for building the project
set_property(TARGET ${Example_Name} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${Example_Name} PROPERTY CXX_STANDARD_REQUIRED ON)

# Define C version to be used for building the project
set_property(TARGET ${Example_Name} PROPERTY C_STANDARD 99)
set_property(TARGET ${Example_Name} PROPERTY C_STANDARD_REQUIRED ON)

set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../engine/vulkan/implementation/VulkanShaders.qrc)

qt5_use_modules(${PROJECT_NAME} Widgets)
