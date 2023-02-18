

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE SOURCES ../src/bonk/*.cpp ../src/bonk/*.hpp ../src/utils/*.cpp ../src/utils/*.hpp *.cpp *.hpp)
add_executable(bonk-tests ${SOURCES})
target_include_directories(bonk-tests PUBLIC "../src")

target_link_libraries(bonk-tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(bonk-tests)