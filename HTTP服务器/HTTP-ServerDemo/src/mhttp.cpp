#include "mhttp.h"
#include "base.h"
#include <cstdio>
#include <string>
#include <cstring>
#include <unistd.h>



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