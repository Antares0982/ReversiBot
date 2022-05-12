# 以utf8编译
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# 最低cmake版本
cmake_minimum_required(VERSION 3.5)
# 声明C++项目
project(reversiBotMiraiCP LANGUAGES CXX)
# C++ 标准20
set(CMAKE_CXX_STANDARD 20)
# 在64位编译
set(BUILD_USE_64BITS on)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(MIRAI_CPP_PATH $ENV{MIRAI_CPP_PATH})

message("MiraiCP source code at ${MIRAI_CPP_PATH}")

# 寻找编译的文件
file(GLOB USER_LIBS_PATH ./reversi/*.cpp)
file(GLOB MIRAI_CPP_FILE ${MIRAI_CPP_PATH}/*.cpp)
file(GLOB MIRAI_H_FILE ${MIRAI_CPP_PATH}/*.h*)

# 添加include
include_directories(${MIRAI_CPP_PATH})
include_directories(${MIRAI_CPP_PATH}/../3rd_include)
include_directories(${MIRAI_CPP_PATH}/../3rd_include/utf8)
include_directories(${MIRAI_CPP_PATH}/../3rd_include/utf8/utf8)

# 构建动态链接库
add_library(
        reversiBotMiraiCP
        SHARED
        main.cpp
        ${USER_LIBS_PATH}
        ${MIRAI_CPP_FILE}
        ${MIRAI_H_FILE}
)

# JNI
find_package(JNI REQUIRED)
if (JNI_FOUND)
    message(STATUS "FOUND JNI IN ${JAVA_INCLUDE_PATH}")
    set(SOURCE_PATH
            ${CMAKE_CURRENT_SOURCE_DIR}/3rd_include/
            ${CMAKE_CURRENT_SOURCE_DIR}/3rd_include/utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/3rd_include/utf8/utf8
            ${JAVA_INCLUDE_PATH}/
            ${JAVA_INCLUDE_PATH2}/
            )
    target_include_directories(reversiBotMiraiCP PUBLIC ${SOURCE_PATH})
else ()
    message(STATUS "Cannot find JNI")
endif ()

# 设置输出的dll文件的名字(最后一个参数)，Linux上是libReversiBotMiraiCP.so
set_target_properties(reversiBotMiraiCP PROPERTIES OUTPUT_NAME ReversiBotMiraiCP)

target_link_libraries(reversiBotMiraiCP ${OpenCV_LIBS})

target_compile_options(reversiBotMiraiCP PUBLIC -pthread -Wall -Og)
