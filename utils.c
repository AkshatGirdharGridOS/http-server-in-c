#include <winsock2.h> // windows library to interact with sockets
#include <windows.h>  // to interact with the windows apis
#include <ws2tcpip.h> // TCP/IP specific extensions in Windows Sockets 2
#include <iphlpapi.h> // Ip address related operations

#include <stdio.h>  // standard I/O
#include <time.h>   // types,definitions and function declarations related to date and time.
#include <stdlib.h> // standard c library
#include <string.h> // library for string operations etc.

#include "config.h"

// Enhanced error logging
void log_error(const char *message)
{
    time_t now; // variable that will store the current time
    time(&now); // get the current time and store it in the variable now
    // printing the error message to the standard error stream
    // format of the message is defined : [timestamp] ERROR : message
    fprintf(stderr, "[%s] ERROR : %s \n", ctime(&now), message);
}

// HTTP response generation
void send_http_response(SOCKET client, int status_code, const char *content_type, const char *body, size_t body_length)
{
    char response[BUFF_LEN];
    char status_message[64];

    // we should map the status codes to messages
    switch (status_code)
    {
    case 200:
        strcpy(status_message, "OK");
        break;
    case 404:
        strcpy(status_message, "Not found");
        break;
    case 500:
        strcpy(status_message, "Internal Server error");
        break;
    default:
        strcpy(status_message, "Unknown error");
        break;
    }

    // then we will contruct an actual http response that the server will send using snprintf
    int header_length = snprintf(response, sizeof(response),
                                 "HTTP/1.1 %d %s\r\n"
                                 "Content-Type: %s\r\n"
                                 "Content-Length: %zu\r\n"
                                 "Connection: close\r\n"
                                 "\r\n",
                                 status_code, status_message, content_type, body_length);

    // these are the headers, we will send the headers first
    send(client, response, header_length, 0);

    size_t total_sent = 0;
    while (total_sent < body_length)
    {
        int bytes_sent = send(client, body + total_sent, body_length - total_sent, 0);
        if (bytes_sent <= 0)
        {
            log_error("Failed to send response body.");
            break;
        }
        total_sent += bytes_sent;
    }
}

// File reading with error handling
int read_file(const char *filename, char **output, size_t *file_size)
{
    FILE *file = fopen(filename, "rb"); // reading the file in windows, uses rb
    if (!file)
    {
        log_error("Could not open file");
        return -1;
    }

    // then we will get the file size after opening it in the rb mode
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    // allocate  memory
    *output = malloc(*file_size + 1);
    if (!*output)
    {
        log_error("Memory allocation failed.");
        if (file)
        {
            fclose(file);
        }; // close the file if output is not found
        return -1;
    }

    // read the file content into the memory
    size_t read_size = fread(*output, 1, *file_size, file);
    if (read_size != *file_size)
    {
        log_error("File read error, file size and read size unequal!");
        free(*output);
        fclose(file);
        return -1;
    }

    (*output)[*file_size] = '\0';
    fclose(file);
    return 0; // if everything is successful then it returns 0.
}

// simple request parsing mechanism
int parse_request(char *request, char *method, char *path)
{
    // simple request parsing
    if (sscanf(request, "%s %s", method, path) != 2)
    {
        return -1;
    }
    return 0;
}

char *extract_body(const char *request)
{
    char *body = strstr(request, "\r\n\r\n");
    return body ? body + 4 : NULL;
}

// defining helper functions which will be used further in the code
void cleanup(SOCKET listener)
{
    if (listener && listener != INVALID_SOCKET)
    {
        closesocket(listener);
    }
    WSACleanup();
};
