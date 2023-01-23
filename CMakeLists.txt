# 最低cmake版本
cmake_minimum_required(VERSION 3.14)
# 声明C++项目
project(reversiBotMiraiCP LANGUAGES CXX)
# C++ 标准20
set(CMAKE_CXX_STANDARD 20)
# 在64位编译
set(BUILD_USE_64BITS on)
# json库启用C++17标准
add_compile_definitions(JSON_HAS_CPP_17 MIRAICP_LIB_SDK JSON_MultipleHeaders=ON)

if (WIN32)
    # See Warning C4668
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif (WIN32)

# MSVC设置
if (MSVC)
    SET(CMAKE_GENERATOR_PLATFORM x64)
    # C++17标准；以utf8编译；异常处理模型：标准 C++ stack unwinding；启用多线程编译；禁止无返回值的函数；禁用两个与dll export相关的warning；加入__cplusplus宏支持
    add_compile_options(/utf-8 /EHa /MP /we4715 /wd4251 /wd4275 /Zc:__cplusplus)
    # Disable RTTI
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else ()
    # 禁止无返回值的函数
    add_compile_options(-Wall -Werror=return-type)
    # 隐藏符号表
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif (MSVC)

# 优化选项，需要使用cmake -DCMAKE_BUILD_TYPE=Release 命令启用
if (CMAKE_BUILD_TYPE MATCHES ".*Rel.*")
    message("Release build detected, enabling maximal optimization")
    if (MSVC)
        add_compile_options(/Ox)
    else ()
        add_compile_options(-O3)
    endif (MSVC)
endif (CMAKE_BUILD_TYPE MATCHES ".*Rel.*")

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# MiraiCP source
set(MIRAI_CPP_PATH $ENV{MIRAI_CPP_PATH})
set(MIRAI_INCLUDE_PATH ${MIRAI_CPP_PATH}/include)
set(MIRAI_SOURCE_PATH $ENV{MIRAI_CPP_PATH}/src/sdk)
set(MIRAI_COMMON_PATH $ENV{MIRAI_CPP_PATH}/src/common)
#
message("MiraiCP source code at ${MIRAI_SOURCE_PATH}, common include at ${MIRAI_COMMON_PATH}")

# 寻找编译的文件
file(GLOB USER_LIBS_PATH ./reversi/*.cpp)
file(GLOB MIRAI_CPP_FILE ${MIRAI_SOURCE_PATH}/*.cpp)
file(GLOB MIRAI_COMMON_CPP_FILE ${MIRAI_COMMON_PATH}/*.cpp)

# 添加include
include_directories(${MIRAI_INCLUDE_PATH}/sdk)
include_directories(${MIRAI_INCLUDE_PATH}/common)
include_directories(${MIRAI_INCLUDE_PATH}/../3rd_include)
include_directories(${MIRAI_INCLUDE_PATH}/../3rd_include/utf8)
include_directories(${MIRAI_INCLUDE_PATH}/../3rd_include/utf8/utf8)
include_directories(${MIRAI_INCLUDE_PATH}/../3rd_include/json)
include_directories(${MIRAI_INCLUDE_PATH}/../3rd_include/json/nlohmann)

# 构建动态链接库
add_library(
        reversiBotMiraiCP
        SHARED
        main.cpp
        ${USER_LIBS_PATH}
        ${MIRAI_CPP_FILE}
        ${MIRAI_COMMON_CPP_FILE}
)

# 保证插件移植性，注意如果存在无法静态链接的其他依赖，仍然无法移植
set_target_properties(reversiBotMiraiCP PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(reversiBotMiraiCP PROPERTIES LINK_SEARCH_END_STATIC 1)
if (WIN32)
    if (MSVC)
        target_compile_options(reversiBotMiraiCP PUBLIC /MT)
    else ()
        target_link_options(reversiBotMiraiCP PUBLIC -static)
    endif (MSVC)
endif (WIN32)
if (UNIX)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    target_link_options(reversiBotMiraiCP PUBLIC -static-libstdc++ -static-libgcc)
endif (UNIX)

# 设置输出的dll文件名
set_target_properties(reversiBotMiraiCP PROPERTIES OUTPUT_NAME ReversiBotMiraiCP)

target_link_libraries(reversiBotMiraiCP ${OpenCV_LIBS})
