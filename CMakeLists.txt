cmake_minimum_required(VERSION 3.10)
project(modernx CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED)

# Добавляем Xft
find_package(PkgConfig REQUIRED)
pkg_check_modules(XFT REQUIRED xft)

# mruby settings
set(MRUBY_DIR ${CMAKE_SOURCE_DIR}/../mruby)
set(MRUBY_INCLUDE_DIR ${MRUBY_DIR}/include)
set(MRUBY_LIB ${MRUBY_DIR}/build/host/lib/libmruby.a)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${X11_INCLUDE_DIR}
    ${XFT_INCLUDE_DIRS}  # Xft
    ${MRUBY_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${X11_LIBRARIES}
    ${XFT_LIBRARIES}  # Xft
    ${MRUBY_LIB}
    m
)