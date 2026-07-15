add_executable(mudcore_tests
    gmcp_parser_test.cpp
    connection_lifecycle_test.cpp
    event_bus_test.cpp
    game_state_test.cpp
    map_overlay_test.cpp
    inbound_pipeline_test.cpp
    outbound_pipeline_test.cpp
    session_test.cpp
)

target_link_libraries(mudcore_tests
    PRIVATE
        mudcore
        test_helpers
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mudcore_tests)
