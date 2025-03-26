#include "config.h"
#include <winsock2.h> // windows library to interact with sockets
#include <windows.h>  // to interact with the windows apis
#include <ws2tcpip.h> // TCP/IP specific extensions in Windows Sockets 2
#include <iphlpapi.h> // Ip address related operations

#include <stdio.h>  // standard I/O
#include <time.h>   // types,definitions and function declarations related to date and time.
#include <stdlib.h> // standard c library
#include <string.h> // library for string operations etc.

#include "utils.h"

// Client handler function for each thread
DWORD WINAPI handle_client(LPVOID lpParam)
{
    SOCKET client = *(SOCKET *)lpParam;
    char recvbuffer[BUFF_LEN];
    char method[16], path[64];

    int recv_result = recv(client, recvbuffer, BUFF_LEN - 1, 0);

    if (recv_result == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        if (error_code == WSAETIMEDOUT) {
            log_error("Recv operation timed out.");
        } else {
            log_error("Recv Failed");
        }
        shutdown(client,SD_BOTH);
        closesocket(client);
        return 1;
    }

    if (recv_result > 0)
    {
        recvbuffer[recv_result] = '\0'; // placing the terminating \0 manually after the buffer since it won't be present in the raw string.
        printf("Received HTTP request : \n %s \n ", recvbuffer);

        if (parse_request(recvbuffer, method, path) == 0)
        {
            // Handling only GET reqs
            if (strcmp(method, "GET") == 0)
            {
                char *file_content;
                size_t file_size;
                if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0)
                {
                    if (read_file("index.html", &file_content, &file_size) == 0)
                    {
                        send_http_response(client, 200, "text/html", file_content, file_size);
                        free(file_content);
                    }
                    else
                    {
                        send_http_response(client, 404, "text/plain", "File Not Found", strlen("File Not Found"));
                    }
                }
                else
                {
                    if (read_file("404.html", &file_content, &file_size) == 0)
                    {
                        send_http_response(client, 404, "text/html", file_content, file_size);
                        free(file_content);
                    }
                    else
                    {
                        send_http_response(client, 404, "text/plain", "File not Found for 404", strlen("File not Found for 404"));
                    }
                }
            }
            // Handling POST reqs
            else if (strcmp(method, "POST") == 0)
            {
                char *body = extract_body(recvbuffer);
                if (body)
                {
                    printf("Received POST body : \n %s \n", body);
                    send_http_response(client, 200, "text/plain", body, strlen(body));
                }
                else
                {
                    send_http_response(client, 400, "text/plain", "Bad req - No Body Found", strlen("Bad req - No Body Found"));
                }
            }
            else
            {
                send_http_response(client, 500, "text/plain", "Method not allowed", strlen("Method not allowed"));
            }
        }
        else if (recv_result == 0)
        {
            printf("Client disconnected gracefully\n");
        }
        else
        {
            log_error("recv failed.");
        }
    }

    shutdown(client, SD_BOTH);
    closesocket(client);
    return 0;
}