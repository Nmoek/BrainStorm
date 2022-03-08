cmake_minimum_required(VERSION 3.0)
project(BrainStorm)



set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


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

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)