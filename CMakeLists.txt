
cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET 13 CACHE STRING "Minimum OS X deployment version.")

set(OPENFOAM_VERSION 2312)

set(PROJECT_NAME OpenFOAM)
set(_PROJECT_VERSION 10.0.0)
set(PROJECT_DEVELOPER_VERSION )
project(${PROJECT_NAME} VERSION ${_PROJECT_VERSION} LANGUAGES CXX C)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules)

option(OPENFOAM_SHARED_LIBS "Build shared libraries" ON)
set(BUILD_SHARED_LIBS ${OPENFOAM_SHARED_LIBS})

set(CMAKE_CXX_STANDARD 14)
set(WM_LABEL_SIZE 32)
set(WM_PRECISION DP)
set(WM_VERSION OPENFOAM=${OPENFOAM_VERSION})

find_package(ZLIB REQUIRED)
find_package(FLEX REQUIRED)
find_package(FFTW3f REQUIRED)

find_package(RAGEL)
find_package(METIS)
find_package(MPI)
find_package(SCOTCH)

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(USING_CLANG TRUE)
endif()

if (TARGET SCOTCH::ptscotch)
  set(PTSCOTCH_FOUND 1)
endif()

if (MPI_FOUND)
  string(FIND "${MPI_C_INCLUDE_DIRS}" "mpich" _MPICH_INDEX)
  if (_MPICH_INDEX EQUAL -1)
    set(MPI_OPTION OpenMPI)
  else()
    set(MPI_OPTION MPICH)
  endif()
else()
  set(MPI_OPTION DummyMPI)
endif()

add_subdirectory(cmakemodules/lemon)

include(GenerateLemon)

set(CMAKE_CXX_FLAGS "-ftemplate-depth-100 -Wall -Wextra -Wold-style-cast -Wnon-virtual-dtor -Wno-unused-parameter -Wno-invalid-offsetof -Wno-undefined-var-template -Wno-old-style-cast")

set(BUILD git-repo)
set(PATCH 1)
set(VERSION ${_PROJECT_VERSION})

set(WM_LABEL_OPTION Int${WM_LABEL_SIZE})
set(WM_OPTIONS "${CMAKE_CXX_PLATFORM_ID}${CMAKE_CXX_COMPILER_ID}${WM_PRECISION}${WM_LABEL_OPTION}${MPI_OPTION}${CMAKE_BUILD_TYPE}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/platforms/${WM_OPTIONS}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/platforms/${WM_OPTIONS}/bin")

add_subdirectory(src)

add_subdirectory(applications)

export(EXPORT openfoam-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/openfoam-targets.cmake"
       NAMESPACE OpenFOAM::
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/openfoam-config.in.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/openfoam-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenFOAM
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/openfoam-config-version.cmake"
  VERSION "${_PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

if (APPLE)
  set(_MACOS_PREFIX DY)
endif()

set(_ENV_SETTINGS "
export WM_PROJECT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
export WM_OPTIONS=${WM_OPTIONS}
export WM_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
export ${_MACOS_PREFIX}LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
export PATH=${CMAKE_CURRENT_BINARY_DIR}/platforms/${WM_OPTIONS}/bin:$PATH
")
message("")
message("Environment settings for this build of OpenFOAM:")
message(${_ENV_SETTINGS})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/openfoam-env.sh ${CMAKE_CURRENT_BINARY_DIR}/openfoam-env.sh)

