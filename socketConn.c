#include "socketConn.h"
/*
void *runServer (int* sockfd, int* newfd)
{
    int ret;



    //pthread_t read_tid,write_tid;
    //struct sockaddr_in server_addr;
    //server_addr.sin_family=AF_INET;        设置域为IPV4
    //server_addr.sin_addr.s_addr=INADDR_ANY;绑定到 INADDR_ANY 地址
    //server_addr.sin_port=htons(10002);      通信端口号为5678，注意这里必须要用htons函数处理一下，不能直接写5678，否则可能会连不上
    //sockfd=socket(AF_INET,SOCK_STREAM,0);

    if (*sockfd<0)
    {
        printf("调用socket函数建立socket描述符出错！\n");
        exit(1);
    }

    printf("调用socket函数建立socket描述符成功！\n");
    ret=bind(*sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr));
    perror("server");

    if (ret<0)
    {
        printf("调用bind函数绑定套接字与地址出错！\n");
        exit(2);
    }
    printf("调用bind函数绑定套接字与地址成功！\n");
    ret=listen(*sockfd,4);

    if (ret<0)
    {
        printf("调用listen函数出错，无法宣告服务器已经可以接受连接！\n");
        exit(3);
    }

    printf("调用listen函数成功，宣告服务器已经可以接受连接请求！\n");
    newfd=accept(*sockfd,NULL,NULL);newfd连接到调用connect的客户端

    if (*newfd<0) {
        printf("调用accept函数出错，无法接受连接请求，建立连接失败！\n");
        exit(4);
    }

    printf("调用accept函数成功，服务器与客户端建立连接成功！\n");

}
*/