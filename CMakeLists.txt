cmake_minimum_required(VERSION 3.28.3)
project(cflare_ddns)
project(cflare_ddns LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set windows windows options
if(WIN32)
    set(MSYS_INCLUDE "C:/msys64/ucrt64/")
    set(MSYS_LIB "C:/msys64/ucrt64/lib")
    list(PREPEND CMAKE_PREFIX_PATH "${MSYS_INCLUDE}")
endif ()

# Find required packages
find_package(jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB QUIET)
find_package(CURL REQUIRED)

# Add executable
add_executable(cflare_ddns
        src/cflare.cpp
        src/cflare.h
        src/Logger.h
        src/main.cpp
        src/Logger.cpp
)


# Link libraries
target_link_libraries(cflare_ddns
        CURL::libcurl
        JsonCpp::JsonCpp
        OpenSSL::SSL
        OpenSSL::Crypto
)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if (WIN32)
        target_compile_options(cflare_ddns PRIVATE -mwindows -O2 -Wall -Wextra -Werror)
    endif ()
    target_compile_options(cflare_ddns PRIVATE -O2 -Wall -Wextra -Werror)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_options(cflare_ddns PRIVATE  -fsanitize=address,undefined )
    target_compile_options(cflare_ddns PRIVATE -O0 -Wall -Wextra -g -fsanitize=address,undefined -fno-omit-frame-pointer)

endif()
