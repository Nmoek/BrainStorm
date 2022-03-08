#include <iostream>

#include "myserver.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <errno.h>


#define THREAD_NUM  5

int main()
{
    BrainStorm::MyServer::ptr s(new BrainStorm::MyServer(THREAD_NUM));
    if(s->listen(9999) < 0)
    {
        return 0;
    }

    s->start();


    return 0;
}
