cmake_minimum_required(VERSION 3.16)

project(TheLimitOfDreams LANGUAGES CXX)

# Ensure C++17 for compatibility with Urho3D headers
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If URHO3D_DIR is not provided, prefer the local rbfx shared install created earlier.
if(NOT DEFINED URHO3D_DIR)
	# Prefer the local rbfx shared install (shared build) if available
	set(_default_urho_dir "${CMAKE_SOURCE_DIR}/../rbfx/build/install/share/Urho3D/CMake")
	if(EXISTS "${_default_urho_dir}/Urho3DConfig.cmake")
		set(URHO3D_DIR "${_default_urho_dir}" CACHE PATH "Path to Urho3DConfig.cmake")
		message(STATUS "Using local Urho3D from: ${URHO3D_DIR}")
	endif()
endif()

# Also add the install prefix to CMAKE_PREFIX_PATH so find_package can locate the config.
set(_default_urho_prefix "${CMAKE_SOURCE_DIR}/../rbfx/build/install")
if(EXISTS "${_default_urho_prefix}/share/Urho3D/CMake/Urho3DConfig.cmake")
	list(APPEND CMAKE_PREFIX_PATH "${_default_urho_prefix}")
	message(STATUS "Appended ${_default_urho_prefix} to CMAKE_PREFIX_PATH")
	# Prefer shared Urho3D imported targets from the local install
	set(URHO3D_SHARED ON CACHE BOOL "Use Urho3D shared build" FORCE)
endif()

find_package(Urho3D REQUIRED)

file(GLOB_RECURSE GAME_SOURCES Source/*.cpp)

add_executable(TheLimitOfDreams ${GAME_SOURCES})

target_link_libraries(TheLimitOfDreams PRIVATE Urho3D)

target_include_directories(TheLimitOfDreams PRIVATE Source)

# Post-build: create convenient `Data` directory in build pointing to engine data
# Uses first entry of CMAKE_PREFIX_PATH as Urho3D install prefix if available.
if(CMAKE_PREFIX_PATH)
	list(GET CMAKE_PREFIX_PATH 0 URHO3D_PREFIX)
else()
	set(URHO3D_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif()

add_custom_command(TARGET TheLimitOfDreams POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Data
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/Techniques
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/Shaders
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/Models
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/Materials
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/Textures
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/RenderPaths
	COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Data/UI
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/Techniques" "${CMAKE_BINARY_DIR}/Data/Techniques"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/Shaders" "${CMAKE_BINARY_DIR}/Data/Shaders"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/Models" "${CMAKE_BINARY_DIR}/Data/Models"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/Materials" "${CMAKE_BINARY_DIR}/Data/Materials"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/Data/Textures" "${CMAKE_BINARY_DIR}/Data/Textures"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/RenderPaths" "${CMAKE_BINARY_DIR}/Data/RenderPaths"
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${URHO3D_PREFIX}/bin/CoreData/UI" "${CMAKE_BINARY_DIR}/Data/UI"
)
