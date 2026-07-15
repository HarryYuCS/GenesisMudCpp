include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

add_library(mudcore STATIC)

target_sources(mudcore
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/connection_life_cycle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gmcp_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game_state.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/map_overlay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/event_bus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/inbound_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/outbound_pipeline.cpp
)

target_include_directories(mudcore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mudcore
    PUBLIC
        network
        nlohmann_json::nlohmann_json
)
