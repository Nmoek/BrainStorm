#ifndef COMMON_H
#define COMMON_H
/*---------------------用户通信协议----------------------*/
namespace BrainStorm
{

// 用户操作指令 5xxx
#define     OK                          5001    //操作成功
#define     REG                         5002    //注册   
#define     LOG                         5003    //登录
#define     SINGLEGETQUESTION           5004    //单机答题
#define     COMPETE                     5005    //匹配
#define     COMPETEGETQUESTION          5006    //匹配答题
#define     ANSWER                      5007    //作答
#define     COMPETERESULT               5008    //匹配结果


/*错误提示 指令8xxx*/
#define     ERROR                       8001    //未知错误
#define     USEREXIST                   8002    //用户已经存在
#define     NAMEORPASS                  8003    // 用户或 密码错误
#define     USERLOGREADY                8004    //用户已经登陆


}

#endif // COMMON_H
