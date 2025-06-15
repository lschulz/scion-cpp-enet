cmake_minimum_required(VERSION 3.28)

find_package(Curses)

add_executable(scion-enet-demo
    "demo/main.cpp"
    "${CMAKE_SOURCE_DIR}/examples/include/console.cpp"
)

target_link_libraries(scion-enet-demo PRIVATE
    scion-cpp
    CLI11::CLI11
    "${CURSES_LIBRARIES}"
)

target_include_directories(scion-enet-demo PRIVATE
    "${CMAKE_SOURCE_DIR}/examples/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CURSES_INCLUDE_DIR}"
)
