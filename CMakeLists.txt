cmake_minimum_required(VERSION 3.26)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
endif()

project(
	BehaviorDataInjector
	VERSION 0.1.7
	LANGUAGES CXX
)

set(SKSEPluginAuthor "max-su-2019; universal compatibility build by DeadOnKeyboard1, 1.7.104 verified")
set(_bdi_toolchain_default "${CMAKE_SOURCE_DIR}/../_toolchain")
if(EXISTS "${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG/CMakeLists.txt")
	set(_bdi_commonlib_default "${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG")
else()
	set(_bdi_commonlib_default "${_bdi_toolchain_default}/CommonLibSSE-NG-v12.1.0")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/extern/DKUtil/CMakeLists.txt")
	set(_bdi_dkutil_default "${CMAKE_SOURCE_DIR}/extern/DKUtil")
elseif(EXISTS "${_bdi_toolchain_default}/DKUtil-0ddab238/CMakeLists.txt")
	set(_bdi_dkutil_default "${_bdi_toolchain_default}/DKUtil-0ddab238")
else()
	set(_bdi_dkutil_default "${_bdi_toolchain_default}/DKUtil")
endif()
set(CommonLibPath "${_bdi_commonlib_default}" CACHE PATH "Path to CommonLibSSE-NG")
set(DKUtilPath "${_bdi_dkutil_default}" CACHE PATH "Path to DKUtil")

# Universal Flatrim SE/AE configuration (Skyrim VR is explicitly rejected at load).
set(ENABLE_SKYRIM_SE ON CACHE BOOL "Enable Skyrim SE support" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "Enable Skyrim AE support" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "Enable Skyrim VR support" FORCE)
set(COMMONLIB_PREBUILT OFF CACHE BOOL "Build CommonLibSSE-NG from source with privacy-safe flags" FORCE)

# If the prebuilt cannot be used, fall back to a source build without CommonLib's unit tests.
set(BUILD_TESTS OFF CACHE BOOL "Disable CommonLibSSE-NG tests for consumer build" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BDI_PATHMAP_ROOT "X:/workspace" CACHE STRING "Neutral deterministic debug source prefix")
if(MSVC)
	add_compile_options("/experimental:deterministic")
	if(DEFINED ENV{USERPROFILE})
		file(TO_NATIVE_PATH "$ENV{USERPROFILE}" USERPROFILE_NATIVE)
		add_compile_options(
			"/pathmap:${USERPROFILE_NATIVE}=X:/user"
			"/d1trimfile:${USERPROFILE_NATIVE}\\"
		)
	endif()
	file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" SOURCEDIR_NATIVE)
	add_compile_options(
		"/pathmap:${SOURCEDIR_NATIVE}=${BDI_PATHMAP_ROOT}/source"
		"/d1trimfile:${SOURCEDIR_NATIVE}\\"
	)
endif()

if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed.")
endif()

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

set(SOURCES
	src/BDIObject.cpp
	src/BDIObject.h
	src/ConsoleCommands.cpp
	src/ConsoleCommands.h
	src/DataHandler.cpp
	src/DataHandler.h
	src/Hook.cpp
	src/Hook.h
	src/LoadGame.cpp
	src/LoadGame.h
	src/PCH.h
	src/main.cpp
)

if(NOT EXISTS "${CommonLibPath}/CMakeLists.txt")
	message(FATAL_ERROR "CommonLibSSE-NG not found. Run Build-Release.cmd first.")
endif()
if(NOT EXISTS "${DKUtilPath}/CMakeLists.txt")
	message(FATAL_ERROR "DKUtil not found. Run Build-Release.cmd first.")
endif()

add_subdirectory(${CommonLibPath} CommonLibSSE EXCLUDE_FROM_ALL)
add_subdirectory(${DKUtilPath} DKUtil EXCLUDE_FROM_ALL)
find_package(spdlog CONFIG REQUIRED)

add_library(
	${PROJECT_NAME}
	SHARED
	${SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		DKUtil::DKUtil
		spdlog::spdlog
)

# DKUtil's default 64-entry enum reflection expansion can trigger an MSVC 19.51 ICE.
# BDI only reflects an enum with five values.
target_compile_definitions(${PROJECT_NAME} PRIVATE SKSEAPI DKU_MAX_REFLECTION_ENUM=16)

if(MSVC)
	add_compile_definitions(_UNICODE)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:Release>:/Zi>"
			/MP
			/W0
			/WX
			/permissive-
			/Zc:alignedNew
			/Zc:auto
			/Zc:__cplusplus
			/Zc:externC
			/Zc:externConstexpr
			/Zc:forScope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept
			/Zc:lambda
			/Zc:noexceptTypes
			/Zc:preprocessor
			/Zc:referenceBinding
			/Zc:rvalueCast
			/Zc:sizedDealloc
			/Zc:strictStrings
			/Zc:ternary
			/Zc:threadSafeInit
			/Zc:trigraphs
			/Zc:wchar_t
			/wd4200
	)
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:Release>:/DEBUG:FULL>"
			"/PDBALTPATH:%_PDB%"
			"/Brepro"
	)

	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES
			PDB_NAME "BehaviorDataInjector"
			COMPILE_PDB_NAME "BehaviorDataInjector"
	)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME "BehaviorDataInjector"
)
