#ifndef _UTIL_H_
#define _UTIL_H_

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <execinfo.h>     //返回函数栈



namespace BrainStorm
{

/**
 * @brief 获取当前线程ID
 * @return 返回PID值
 */
pid_t GetThreadId();

/**
 * @brief 获取函数调用栈
 * @param[out] bt 传出已经获取的函数调用栈信息 
 * @param[in] size 期望获取的函数栈层数上限
 * @param[in] skip 选择跳过显示的层数
 */
void BackTrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 打印函数调用栈信息
 * @param[in] size 期望获取的函数栈层数上限
 * @param[in] prefix 打印格式控制
 * @param[in] skip 选择跳过显示的层数
 * @return 返回函数调用栈信息的字符串
 */
std::string BackTraceToString(int size = 64, const std::string &prefix = "", int skip = 2);


/**
 * @brief 获取ms级精度时间
 * @return 返回时间值
 */
uint64_t GetCurrentMs();


/**
 * @brief 获取us级时间
 * @return 返回时间值
 */
uint64_t GetCurrentUs();

/**
 * @brief 将时间戳转换为字符串形式
 * @param ts  时间戳数值
 * @param format 转换为具体的字符串时间格式
 * @return 返回时间字符串
 */
std::string Timer2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");


}

#endif