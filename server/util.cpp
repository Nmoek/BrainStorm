#include "util.h"

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <fstream>
#include <signal.h>
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace BrainStorm
{

static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}

//获取函数堆栈
void BackTrace(std::vector<std::string>& bt, int size, int skip)
{
    void ** arr = (void**)malloc(sizeof(void *) * size);

    //返回当前堆栈使用的真实层数  size是期望层数上限
    size_t true_size = backtrace(arr, size);

    //这里面有malloc的过程 strings是一块动态分配内存
    char **strings = backtrace_symbols(arr, true_size);
    if(strings == nullptr)
    {
        g_logger->error("BackTrace: backtrace_symbols error");
        return;
    }

    for(size_t i = skip;i < true_size;++i)
    {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(arr);
}


//打印函数堆栈
std::string BackTraceToString(int size, const std::string &prefix, int skip)
{
    std::vector<std::string> bt;
    BackTrace(bt, size, skip);

    //借助流 输出
    std::stringstream ss;
    for(size_t i = 0;i < bt.size();++i)
    {
        ss << prefix << bt[i] << std::endl;
    }

    return ss.str();
}



//获取ms级时间
uint64_t GetCurrentMs()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

//获取us级时间
uint64_t GetCurrentUs()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

std::string Timer2Str(time_t ts, const std::string& format)
{
    struct tm tm;
    tm = *localtime(&ts);
    char buf[100];
    strftime(buf, sizeof(buf), format.c_str(), &tm);

    return buf;
}


}