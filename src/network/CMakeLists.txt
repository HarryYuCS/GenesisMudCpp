add_library(network STATIC)

target_sources(network
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/telnet_codec.cpp
)

target_include_directories(network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(network
    PUBLIC
        Boost::headers
        Boost::system
)
