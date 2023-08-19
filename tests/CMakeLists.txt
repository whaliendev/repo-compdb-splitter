add_executable(runUnitTests
    main.cpp
    trie_test.cpp)

target_link_libraries(runUnitTests
    gtest)

add_test(
    NAME UnitTests
    COMMAND ${PROJECT_BINARY_DIR}/bin/runUnitTests
)