#ifndef _MACRO_H_
#define _MACRO_H_

/*
*   自己定义一些后续方便调试的宏定义
*/

#include <string.h>
#include <assert.h>
#include "util.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>



#define KIT_ASSERT(x) do{\
    if(!(x)){\
        spdlog::get("BrainStorm")->error("\nASSERTION: " #x   \
                                        "\nbacktrace:{} \n" \
                                      ,BrainStorm::BackTraceToString(100, "    "));\
        assert(x);\
    }\
}\
while(0);

//第二个参数可做一些补充说明
#define KIT_ASSERT2(x, w)do{\
    if(!(x)){\
        spdlog::get("BrainStorm")->error ("\nASSERTION: " #x   \
                                       "\n{}, \nbacktrace:{} \n", w\
                                        ,BrainStorm::BackTraceToString(100, "    "));\
        assert(x);\
    }\
}\
while(0)

#endif