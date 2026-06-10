add_library(mudcore STATIC)

target_sources(mudcore
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gmcp_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game_state.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/event_bus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/input_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/output_pipeline.cpp
)

target_include_directories(mudcore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mudcore
    PUBLIC
        network
)
