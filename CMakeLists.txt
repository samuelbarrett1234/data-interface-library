cmake_minimum_required(VERSION 3.26)

project(data-interface-example VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(COMPILER_VERSION_STRING "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(
    data-interface-example
    "graph/examples/graph_dense.cpp"
)

target_include_directories(
    data-interface-example PUBLIC
    ${PROJECT_SOURCE_DIR}
)

if (MSVC)

    target_compile_options(
        data-interface-example PUBLIC

        /W4 /WX

        /external:anglebrackets /external:W0

        /arch:AVX2
    )

else()

    target_compile_options(
        data-interface-example PUBLIC

        -Werror -Wall -Wpedantic -Wextra
        -Wdouble-promotion -Wconversion
        -Wshadow -Wmismatched-tags

        -march=native -mavx2 -mfma
    )

endif()
