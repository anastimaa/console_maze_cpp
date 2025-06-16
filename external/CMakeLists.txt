include(FetchContent)
FetchContent_Declare(asio GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git GIT_TAG asio-1-30-2)
FetchContent_MakeAvailable(asio)
FetchContent_Declare(doctest GIT_REPOSITORY https://github.com/doctest/doctest.git)
FetchContent_MakeAvailable(doctest)
add_library(external_deps INTERFACE)
target_include_directories(external_deps INTERFACE
    ${asio_SOURCE_DIR}/asio/include
    ${doctest_SOURCE_DIR}
)
