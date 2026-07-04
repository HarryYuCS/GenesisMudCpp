include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(test_helpers INTERFACE)
target_include_directories(test_helpers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/helpers)

add_subdirectory(network)
add_subdirectory(mudcore)
