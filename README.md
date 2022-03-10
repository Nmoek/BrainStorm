# BrainStorm
一个即时答题对战项目。包含用户注册、用户登录、单人答题、两人匹配对战答题功能。

# 服务器端
- **环境**：Ubuntu20.04 + vscode
- **编译**： 
  - 服务器源码在`/server/`文件夹路径下，配备`CMakeLists.txt`、`Makefile`拉取源码后为服务器创建一个工作目录，如`BraStorm`。  
  - 在工作目录下执行`cmake .`
  - 执行`make`完成服务器代码编译
  - 运行程序出现在`工作目录/bin/`下， 执行`./main`服务器启动  
 
- **技术点**：
1. **整体架构：** 采用"one loop pre thread"的思想，是一种多reactor模型，借助libevent框架实现, 每个线程都持有libevnt的句柄。主线程负责接收新的客户端连接，将其分发给子线程去和客户端进行数据交互。
  分发连接时进行了简单的负载均衡，每一次接收到新连接依次轮询给子线程分发，这样保证每一个子线程拿到的客户端连接数量比较均匀，避免一个线程中因为有过多的客户连接而产生巨大的负载。
  （可改进点：应该是根据每一个线程的具体和客户端的通繁忙程度来分发，而不是盲目"平均"）
![image](https://user-images.githubusercontent.com/53595455/157326345-24f05eb9-1289-4932-8da1-c42e67294c91.png)


2. **libevent框架使用**：底层封装的就是epoll, 机制原理：套接字加入到epoll + 绑定事件回调 + 异步IO处理。
- `event_base_new()` 初始化libevnt/事件表
  - `event_base_dispatch()` 开启事件循环
  - `event_base_free()` 释放事件表空间
- `event_new()` 创建新的事件，绑定对应事件的回调函数
- `event_add()` 将绑定好的事件添加到事件表
- `bufferevent_socket_new()` 创建libevent缓冲区，绑定套接字，负责触发回调以及数据的收发
  - `bufferevent_setcb()` 设置回调函数
  - `bufferevent_enable()` 使能缓冲区 可读/可写/可接受信号 EV_READ | EV_WRITE | EV_SIGNAL  
  - `bufferevent_read()` 通过缓冲区接收数据
  - `bufferevent_write()` 通过缓冲区发送数据

3. **日志**： 采用C++封装的`spdlog`轻量日志库，支持日志级别、时间日期、线程ID、日志名称和消息体等的输出，支持多日志多地点输出，具备线程安全。使用起来非常的简便。  
![image](https://user-images.githubusercontent.com/53595455/157327801-4d2ad81a-7f37-485c-9192-1c2af9ab164c.png)


4. **MySQL数据库**: 库名：`BrainStorm`, 采用两张表：用户表`user` 和 题目表`question`。用户注册和用户登录都he用户表`user`进行交互。当客户端对题目发起请求时，
  会从题目表`question`中随机的查询出规定数量`QUESTION_NUM`的题目，一并返回给客户端，随机查询的sql语句`"(select * from question order by rand() limit xx"` xx为题目量。
  (rand()函数的查询效率不高，这里随机查询还有改进的余地....)  
- user:  `name`字段作为主键， （用户名  varcahr(255)，密码varvhar(255)，rank分(int) ）
```
             ---------------------------------
             |   name  |   pass    |   rank  |
             ---------------------------------
```
- question: `id`字段为自增主键  除了id字段，其他字段都是 varchar(500)
```
-----------------------------------------------------------------------------------------
|  id  |    question  |   optinA  |   optinA  |   optinA    |   optinA    |   answer    |
-----------------------------------------------------------------------------------------
```

5. **数据格式**：采用`Json`数据格式。先发送数据长度，收到数据长度之后，在发送封装好的Json数据包。由于C++和Qt对Json库都比较完善，因此使用起来比较的方便。
  支持基础数据类型存储、递归存储（Json里还能再存Json）
  
  # 客户端
  - **环境**： Windows + Qt5
  - **技术点** Qt编程 + UI设计 + 信号与槽机制
  - **编译**：将文件夹`client`通过Qt5程序加载`Inport`，建议使用`MinGW-64bit`编译器编译。
----
  - **登录界面**    
  ![image](https://user-images.githubusercontent.com/53595455/157330017-c0ae2d85-58f6-4205-aaf1-679284b87c25.png)    
  ---
  - **注册界面**   
  ![image](https://user-images.githubusercontent.com/53595455/157330108-1486ea68-fe3a-4368-b30b-db3d4b0678b0.png)  
---

  - **主菜单**   
    ![image](https://user-images.githubusercontent.com/53595455/157330868-89139e3b-6727-414c-a6f3-d07482aee8de.png)  
---

  - **单人答题**   
 ![image](https://user-images.githubusercontent.com/53595455/157330974-d9333ec5-e515-4de9-beed-bcb7f28b0872.png)    
 ![image](https://user-images.githubusercontent.com/53595455/157331011-ba019659-fe20-4417-8685-0f976605403b.png)     
![image](https://user-images.githubusercontent.com/53595455/157331050-0c3d58b7-c804-4496-a34b-6b4f888b45fc.png)

---
- **双人匹配答题**   
 ![image](https://user-images.githubusercontent.com/53595455/157331165-5ee07b3d-9bc1-448e-8177-451858a7bf86.png)  
![image](https://user-images.githubusercontent.com/53595455/157331183-495fe5fe-cf4e-41d0-beb5-9068ef0eb717.png)  
![image](https://user-images.githubusercontent.com/53595455/157331227-11b1f7fc-bbf4-419c-b793-a44a9da5849a.png)  

---
---
# 服务器架构改进
- **问题描述**：压测过程中，总是报错`event_queue_insert_inserted`来自libevnt库内部的一个`assert`断言。一开始判断出来，在主线程`accept`接收到新连接时，往子线程的`event_base`中添加，发生了线程竞争的问题，加锁也不管用。经过多番了解发现libevent无法保证线程安全，现在已经有支持但是使用起来非常不方便。  

- **问题解决**：于是将接收新连接分发的这个**通知过程**进行改进，使用无名管道`pipe`去完成主线程通知子线程处理新连接。由于管道自身是线程安全的，不用考虑加锁，每一个子线程持有一个管道，将管道的读端加入到事件循环中，主线程拿到新的连接就通过该管道的写端把新建立的套接字传给子线程，子线程又去进一步完成和客户端的数据交互通信。

- **改进后压测结果**：
  - 使用两台虚拟机，服务端配置：内存4G 3核  客户端配置：内存8G 3核
  - 服务器绑定1个IP开放100个端口，客户端开3个进程依次轮询连接服务器的100个端口。并且连接建立后，使用回射服务器模型，完成一次数据收发。
  - 平均建立1000条连接花费6000ms左右，极限连接数约80W(没能突破100W有点可惜....)


