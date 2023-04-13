cmake_minimum_required(VERSION 3.0.0)
project(forecaster VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wuninitialized -Wshadow -Wno-unused-result -Werror")

if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_executable(forecaster main.cpp)

add_subdirectory(include)
add_subdirectory(src)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 32fd1453194c37d5136577ee55629b863a42333f)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)

FetchContent_MakeAvailable(cpr json)

add_library(handler include/api_handler.h src/api_handler.cpp)
add_library(weather include/weather.h src/weather.cpp)

target_link_libraries(handler PRIVATE cpr::cpr nlohmann_json::nlohmann_json ITMLparse)
target_link_libraries(weather PRIVATE handler cpr::cpr nlohmann_json::nlohmann_json ITMLparse)
target_link_libraries(forecaster PRIVATE cpr::cpr nlohmann_json::nlohmann_json ITMLparse handler weather)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
