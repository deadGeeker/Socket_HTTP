#ifndef MHTTP__H
#define MHTTP__H


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


#endif