cmake_minimum_required(VERSION 3.20)
project(MazeGame LANGUAGES CXX VERSION 2.3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

add_subdirectory(external)

add_executable(server main.cpp server.cpp dfsmaze.cpp)
target_link_libraries(server PRIVATE external_deps ws2_32)

add_executable(client main_client.cpp client.cpp)
target_link_libraries(client PRIVATE external_deps ws2_32)

add_executable(server_test server_test.cpp server.cpp dfsmaze.cpp)
target_link_libraries(server_test PRIVATE external_deps doctest ws2_32)

add_executable(client_test client_test.cpp client.cpp dfsmaze.cpp)
target_link_libraries(client_test PRIVATE external_deps doctest ws2_32)

add_executable(dfsmaze_test dfsmaze_test.cpp dfsmaze.cpp)
target_link_libraries(dfsmaze_test PRIVATE external_deps doctest ws2_32)

enable_testing()

add_test(NAME server_test COMMAND server_test)
add_test(NAME client_test COMMAND client_test)
add_test(NAME dfsmaze_test COMMAND dfsmaze_test)