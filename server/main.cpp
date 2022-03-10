#include <iostream>

#include "myserver.h"

#define THREAD_NUM  10

int main(int argc, char*argv[])
{
    if(argc < 2)
    {
        std::cout << "usage: bin/main [ip] [port]\n" << std::endl;
	    return 0;
    }

    BrainStorm::MyServer::ptr s(new BrainStorm::MyServer(THREAD_NUM));
    if(s->listen(atoi(argv[2]), argv[1]) < 0)
    {
        return 0;
    }

    s->start();


    return 0;
}
