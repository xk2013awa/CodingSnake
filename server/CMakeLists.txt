cmake_minimum_required(VERSION 3.15)
project(SnakeServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Crow SSL feature for HTTPS support
set(CROW_ENABLE_SSL ON CACHE BOOL "Enable Crow SSL" FORCE)

# Find required packages
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Include FetchContent for dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Optional local dependency source directories to speed up configure time
set(CROW_LOCAL_SOURCE_DIR "" CACHE PATH "Local Crow source directory")
set(JSON_LOCAL_SOURCE_DIR "" CACHE PATH "Local nlohmann_json source directory")

if(CROW_LOCAL_SOURCE_DIR STREQUAL "")
    if(EXISTS "${CMAKE_SOURCE_DIR}/../build/_deps/crow-src/CMakeLists.txt")
        set(CROW_LOCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../build/_deps/crow-src")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/../../build/_deps/crow-src/CMakeLists.txt")
        set(CROW_LOCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../build/_deps/crow-src")
    endif()
endif()

if(JSON_LOCAL_SOURCE_DIR STREQUAL "")
    if(EXISTS "${CMAKE_SOURCE_DIR}/../build/_deps/json-src/CMakeLists.txt")
        set(JSON_LOCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../build/_deps/json-src")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/../../build/_deps/json-src/CMakeLists.txt")
        set(JSON_LOCAL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../build/_deps/json-src")
    endif()
endif()

# Crow web framework - try find_package first, then FetchContent
find_package(Crow QUIET)
if(NOT Crow_FOUND)
    if(CROW_LOCAL_SOURCE_DIR AND EXISTS "${CROW_LOCAL_SOURCE_DIR}/CMakeLists.txt")
        message(STATUS "Crow not found as package, using local source: ${CROW_LOCAL_SOURCE_DIR}")
        FetchContent_Declare(
            crow
            SOURCE_DIR ${CROW_LOCAL_SOURCE_DIR}
        )
    else()
        message(STATUS "Crow not found locally, downloading from GitHub (shallow clone)...")
        FetchContent_Declare(
            crow
            GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
            GIT_TAG v1.2.1.2
            GIT_SHALLOW TRUE
        )
    endif()
    FetchContent_MakeAvailable(crow)
else()
    message(STATUS "Found Crow locally")
endif()

# nlohmann/json - try find_package first, then FetchContent
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    if(JSON_LOCAL_SOURCE_DIR AND EXISTS "${JSON_LOCAL_SOURCE_DIR}/CMakeLists.txt")
        message(STATUS "nlohmann_json not found as package, using local source: ${JSON_LOCAL_SOURCE_DIR}")
        FetchContent_Declare(
            json
            SOURCE_DIR ${JSON_LOCAL_SOURCE_DIR}
        )
    else()
        message(STATUS "nlohmann_json not found locally, downloading from GitHub (shallow clone)...")
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
            GIT_SHALLOW TRUE
        )
    endif()
    FetchContent_MakeAvailable(json)
else()
    message(STATUS "Found nlohmann_json locally")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files (exclude test files)
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*test_.*\\.cpp$")

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    Crow::Crow
    nlohmann_json::nlohmann_json
    Threads::Threads
    SQLite::SQLite3
    OpenSSL::SSL
    OpenSSL::Crypto
)


