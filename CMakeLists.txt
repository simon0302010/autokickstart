cmake_minimum_required(VERSION 3.14)
project(autokickstart VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

file(GLOB SOURCES
    "src/*.c"
)

add_executable(autokickstart ${SOURCES})
target_include_directories(autokickstart PRIVATE src)
target_include_directories(autokickstart PRIVATE ${GTK4_INCLUDE_DIRS})
target_link_directories(autokickstart PRIVATE ${GTK4_LIBRARY_DIRS})
target_link_libraries(autokickstart PRIVATE ${GTK4_LIBRARIES})

install(TARGETS autokickstart
    RUNTIME DESTINATION bin
)
