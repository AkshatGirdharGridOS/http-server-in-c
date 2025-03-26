#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h> // windows library to interact with sockets
#include <windows.h>  // to interact with the windows apis
#include <ws2tcpip.h> // TCP/IP specific extensions in Windows Sockets 2
#include <iphlpapi.h> // Ip address related operations

#include <stdio.h>  // standard I/O
#include <time.h>   // types,definitions and function declarations related to date and time.
#include <stdlib.h> // standard c library
#include <string.h> // library for string operations etc.

#pragma comment(lib, "Ws2_32.lib"); // windows specific

#include "config.h"
#include "handle_client.h"
#include "utils.h"

int main()
{
    printf("Hello World\n");
    int res, sendRes;
    int running; // this maintains the state of the function.
    WSADATA wsaData;
    SOCKET listener, client;
    struct sockaddr_in address, clientAddr;
    char recvbuffer[BUFF_LEN];
    char *inputFileContents;
    int inputFileLength;
    char method[16], path[26];

    // Winsock initialization
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res)
    {
        printf("Startup Failed %d.\n", WSAGetLastError());
        log_error("WSAStartup failed.");
        return 1;
    }

    // setup the actual server socket here
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // defines the config for the socket
    // AF_INET means we will use the IPv4 family
    // SOCK_STREAM provides 2 way,reliable, connection based streams, this works with the TCP server.
    // IPPPROTO means the socket will follow tcp
    if (listener == INVALID_SOCKET)
    {
        log_error("Socket creation failed");
        cleanup(0); // cleanup the socket connection
        return 1;
    }

    // address configuration
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS); // we are converting the ADDRESS (localhost in this case)
    // into the type that can be stored in the s_addr, their expected types/definitions are different
    // after setting up the server we will also bind it
    address.sin_port = htons(PORT);
    res = bind(listener, (struct sockaddr *)&address, sizeof(address));
    if (res == SOCKET_ERROR)
    {
        printf("Binding error.\n");
        log_error("Binding failed");
        cleanup(listener);
        cleanup(client);
        return 1;
    }

    // set the server as a listener
    res = listen(listener, SOMAXCONN);
    if (res == SOCKET_ERROR)
    {
        printf("listener setting up failed\n");
        log_error("Listener setup has failed!");
        cleanup(listener);
        return 1;
    }

    // done setting up the server we can now log the server is started on port statement
    printf("The server has started on %s:%d\n", ADDRESS, PORT);
    running = 1;
    while (running)
    {
        // while we are running we have to accept a client
        // for now we will accept any and every client, now how it should be done
        int clientAddrLen = sizeof(clientAddr);
        client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (client == INVALID_SOCKET)
        {
            printf("Could not accept the client\n");
            log_error("Client accept has failed!");
            continue;
        }

        // we have to get the client information
        printf("Client Connected at : %s:%d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);

        HANDLE threadHandle = CreateThread(NULL, 0, handle_client, &client, 0, NULL);
        if (threadHandle == NULL)
        {
            log_error("Thread creation failed!");
            closesocket(client);
        }
        else
        {
            CloseHandle(threadHandle);
        }
    }
}