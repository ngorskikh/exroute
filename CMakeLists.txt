cmake_minimum_required(VERSION 3.18)
project(exroute)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    link_libraries(Ws2_32.lib)
endif ()

if (${EXROUTE_OPT_SANITIZE_ADDRESS})
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif ()

add_executable(exroute main.cpp CidrBlock.cpp)

enable_testing()
add_custom_target(tests)

add_executable(basic_tests EXCLUDE_FROM_ALL test.cpp CidrBlock.cpp)
add_dependencies(tests basic_tests)
add_test(basic_tests basic_tests)
