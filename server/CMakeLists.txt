cmake_minimum_required(VERSION 3.15)
project(SnakeServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Include FetchContent for dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Crow web framework - try find_package first, then FetchContent
find_package(Crow QUIET)
if(NOT Crow_FOUND)
    message(STATUS "Crow not found locally, downloading from GitHub...")
    FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(crow)
else()
    message(STATUS "Found Crow locally")
endif()

# nlohmann/json - try find_package first, then FetchContent
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found locally, downloading from GitHub...")
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
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


