#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>

#define BUFFSIZE    1024

//出错调用函数
void error_handle(std::string opt, std::string message)
{
    //根据errno值获取失败原因并打印到终端
    perror(opt.c_str());
    std::cout << message << std::endl;
    exit(1);
}

//读取一行数据，返回数据长度
int getFdLine(int fd, char *buf, int size);

//回复客户端400错误
void Response400(int client_sock);
//回复客户端404错误
void Response404(int client_sock);
//回复客户端501错误
void Response501(int client_sock);
//回复客户端500错误
void Response500(int client_sock);
//回复客户端正常 200 状态
void Response200(int client_sock);
//返回客户端请求的文件内容
void FileResponse(int client_sock, const char *FileNamePath);

//http 协议客户端请求数据处理
int http_request(int client_sock);



int main(int argc, char *argv[])
{
    int serv_sock;
    int client_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    socklen_t client_addr_size;
    char message[] = "hello world";

    //判断参数数量，Usage: <port>， 需要在命令行输入服务器接收消息的端口号
    if(argc < 2)
    {
        std::cout << "Usage : " << argv[0] << " <prot>" << std::endl;
        exit(1);
    }

    //创建socket 套接字
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0)
    {
        error_handle("socket", "socket() error.");
    }

    //初始化套接字结构体
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);//选择当前任意网卡
    serv_addr.sin_port = htons(atoi(argv[1]));//设置接收消息的端口号

    //绑定端口
    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error_handle("bind", "bind() error.");
    }

    //监听端口，设置等待队列数量为5
    if(listen(serv_sock, 5) < 0)
    {
        error_handle("listen", "listen() error.");
    }
    int ret = 0;
    client_sock = -1;

    while(1)
    {
        //打印输出等待连接
        std::cout << "Waiting Client.... server : " << serv_sock << " client_socke" << client_sock << std::endl;

        client_addr_size = sizeof(client_addr);
        //等待接收客户端建立连接
        client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if(client_sock < 0)
        {
            error_handle("accept", "accept() error.");
        }
        //accept() 成功建立连接后，服务器就会得到客户端的 IP 地址和端口号。
        //打印客户端 IP 和端口号
        std::cout << "Client IP : " << inet_ntoa(client_addr.sin_addr) << " , port : " << ntohs(client_addr.sin_port) << std::endl;

        //http 请求解析处理
        ret = http_request(client_sock);

        std::cout << "ret = " << ret << " client :" << client_sock << std::endl;
        //关闭TCP连接
        close(client_sock);
        client_sock = -1;
    }

    //关闭socket套接字
    close(serv_sock);

    return 0;
}


//HTTP 数据请求解析函数, 成功回复返回 0, 否则返回 -1
int http_request(int client_sock)
{
    char req_line[BUFFSIZE];

    char method[10];
    char ct[15];
    char file_name[30];


    //请求错误，直接返回结果
    if(getFdLine(client_sock, req_line, sizeof(req_line)) == 0){
        Response501(client_sock);
        return -1;
    }

    if(strstr(req_line, "HTTP/")==NULL){
        //请求消息头信息不正确
        Response501(client_sock);
        return -1;
    }

    //判断方式是否正确
    if(strstr(req_line, "GET") != NULL || strstr(req_line, "POST") != NULL)
    {
        //请求方式正确
        // //判断是 post还是 get
        // if(strstr(req_line, "GET") != NULL)
        // {
        //     //请求方式是 GET

        // }
        // else{
        //     //请求方式是 POST

        // }
        //不论是 POST 还是 GET ，处理方式都一样
        Response200(client_sock);
    }
    else{
        //HTTP 请求方式不是 GET ，有问题
        Response501(client_sock);
        return -1;
    }

    return 0;
}

//读取一行数据，返回数据长度
int getFdLine(int fd, char *buf, int size)
{
    char c, *top = buf;
    int ret = 0;

    while((top - buf) <= size)
    {
        if(read(fd, &c, 1) <= 0)
            break;
        else
            *top++ = c;
        if(c == '\r'){
            //读取到了 '\r' 数据，判断下一个数据是否是 '\n'
            read(fd, &c, 1);
            if(c == '\n')
                *top++ = c;
            else
                *top++ = '\n';
            break;
        }
    }
    //追加结束字符
    *top++ = '\0';
    ret = top - buf;

    return ret;
}

//回复客户端400错误
void Response400(int client_sock)
{
    const char estr[] =
    "HTTP/1.0 400 BAD REQUEST\r\n"
    "Server: wz simple httpd 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<p>你的请求有问题,请检查语法!</p>\r\n";

    write(client_sock, estr, strlen(estr));
}

//回复客户端404错误
void Response404(int client_sock)
{
    const char estr[] =
    "HTTP/1.0 404 NOT FOUND\r\n"
    "Server: wz simple httpd 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html>"
    "<head><title>你请求的界面被查水表了!</title></head>\r\n"
    "<body><p>404: 估计是回不来了</p></body>"
    "</html>";

    write(client_sock, estr, strlen(estr));
}

//回复客户端501错误
void Response501(int client_sock)
{
    const char estr[] =
    "HTTP/1.0 501 Method Not Implemented\r\n"
    "Server: wz simple httpd 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html>"
    "<head><title>小伙子不要乱请求</title></head>\r\n"
    "<body><p>too young too simple, 年轻人别总想弄出个大新闻.</p></body>"
    "</html>";

    write(client_sock, estr, strlen(estr));
}

//回复客户端500错误
void Response500(int client_sock)
{
    const char estr[] =
    "HTTP/1.0 500 Internal Server Error\r\n"
    "Server: wz simple httpd 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html>"
    "<head><title>Sorry </title></head>\r\n"
    "<body><p>最近有点方了!</p></body>"
    "</html>";

    write(client_sock, estr, strlen(estr));
}

//回复客户端正常 200 状态
void Response200(int client_sock)
{
    const char estr[] =
    "HTTP/1.0 200 OK\r\n"
    "Server: simple http server 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n";

    const char html[] =
    "HTTP/1.0 200 OK\r\n"
    "Server: simple http server 1.0\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<body>\r\n"
    "<script type=\"text/javascript\">\r\n"
    "document.write(\"<h1>Hello World!</h1>\")\r\n"
    "</script>\r\n"
    "</body>\r\n"
    "</html>\r\n";

    write(client_sock, html, strlen(html));
}


//返回客户端请求的文件内容
void FileResponse(int client_sock, const char *FileNamePath)
{
    char buf[BUFSIZ];
    printf("path = [%s]\n", FileNamePath);
    FILE *txt = fopen(FileNamePath, "r");

    // 如果文件解析错误, 给它个404
    if(NULL == txt)
        Response404(client_sock);
    else{
        //发送给200的报文头过去, 并发送文件内容过去
        Response200(client_sock);
        while(!feof(txt) && fgets(buf, sizeof(buf), txt))
            write(client_sock, buf, strlen(buf));
        fclose(txt);
    }
}