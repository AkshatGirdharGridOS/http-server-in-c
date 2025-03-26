#ifndef UTILS_H
#define UTILS_H

#include <winsock2.h> // windows library to interact with sockets
#include <windows.h>  // to interact with the windows apis
#include <ws2tcpip.h> // TCP/IP specific extensions in Windows Sockets 2
#include <iphlpapi.h> // Ip address related operations


void log_error(const char *message);
void send_http_response(SOCKET client,int status_code,const char *content_type,const char *body,size_t body_length);
int read_file(const char *filename,char **output,size_t *file_size);
int parse_request(char *request,char *method,char *path);
char *extract_body(const char *request);
void cleanup(SOCKET listener);

#endif