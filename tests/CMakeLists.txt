

add_executable(runtests test.cc)


target_link_libraries(runtests
    PRIVATE
        peer
        gtest
)


gtest_discover_tests(runtests)

