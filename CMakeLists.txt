cmake_minimum_required(VERSION 3.0)
project(BrainStorm)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


include_directories(.)


set(LIB_SRC
    server/myserver.cpp
    server/db.cpp
    server/tcpserver.cpp
    server/tcpsocket.cpp
    server/thread.cpp
    server/user.cpp
    server/util.cpp
)


add_library(BrainStorm SHARED ${LIB_SRC})
   
SET(LIB_LIB
    BrainStorm
    event
    pthread
    mysqlclient
    jsoncpp
)

# 生成可执行文件
add_executable(main server/main.cpp)
add_dependencies(main BrainStorm)
target_link_libraries(main ${LIB_LIB})

# 添加执行文件路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 添加动态库路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)