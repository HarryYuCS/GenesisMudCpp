cmake_minimum_required(VERSION 3.28)
project(genesis_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})

# Bottom-up: network → mudcore → gui → executable
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/network)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/mudcore)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

target_link_libraries(${PROJECT_NAME} PRIVATE gui)
