add_executable(network_tests
    telnet_codec_test.cpp
    tcp_session_test.cpp
)

target_link_libraries(network_tests
    PRIVATE
        network
        test_helpers
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(network_tests)
