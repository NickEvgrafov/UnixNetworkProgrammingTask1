#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv)
{
    if (argc >= 2 && strcmp(argv[1], "--help") == 0)
    {
        printf("usage: sender [ipDest] [<portDest>] [<timeoutms>] [<message>] [packCount] [SO_SNDBUF]\n");
        exit(EXIT_SUCCESS);
    }

    char ipDest[16];
    strcpy(ipDest, "10.70.2.104");
    if (argc >= 2)
        strncpy(ipDest, argv[1], 16);
    printf("destination addr = %s\n", ipDest);

    unsigned portDest = 6666;
    if (argc >= 3)
        portDest = atoi(argv[2]);
    printf("destination port = %d\n", portDest);

    int timeoutms = 1000;
    if (argc >= 4)
        timeoutms = atoi(argv[3]);
    printf("timeout (ms) = %d\n", timeoutms);
    useconds_t timeoutusec = timeoutms * 1000;
    
    char* message = "Hello";
    if (argc >= 5)
        message = argv[4];
    printf("message = %s\n", message);

    int packCount = 1;
    if (argc >= 6)
        packCount = atoi(argv[5]);

    int soSndBuff = 1024;
    if (argc >= 7)
    {
        soSndBuff = atoi(argv[6]);
        soSndBuff = soSndBuff > 1024 ? soSndBuff : 1024; 
    }
    printf("soSndBuff = %d\n", soSndBuff);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sendbuff = 0;
    int optlen = sizeof(sendbuff);
    int res = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);
    if (res == -1)
    {
        fprintf(stderr, "getsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("initial send buffer size = %d\n", sendbuff);
    }

    res = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &soSndBuff, sizeof(soSndBuff));
    if (res == -1)
    {
        fprintf(stderr, "setsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("called setsockopt(... %d ...)\n", soSndBuff);
    }
 
    sendbuff = 0;
    optlen = sizeof(sendbuff);
    res = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);
    if (res == -1)
    {
        fprintf(stderr, "getsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("modified send buffer size = %d\n", sendbuff);
    }
    
    struct sockaddr_in destAddr;
    bzero(&destAddr, sizeof(struct sockaddr_in));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(portDest);
    inet_pton(AF_INET, ipDest, &(destAddr.sin_addr));

    int bufferCapacity = (strlen(message) + 64) * packCount;
    char* buffer = (char *)malloc(bufferCapacity);
    int bufferLen = 0;
    int msgNumber = 1;
    
    while (1)
    {
        bufferLen = 0;
        int i = 0;
        for (; i < packCount; ++i)
        {
            int len = sprintf(buffer + bufferLen, "%s %d\n", message, msgNumber);
            ++msgNumber;
            bufferLen += len;
        }

        printf("sending %d bytes:\n", bufferLen);
        int sent = sendto(sock, buffer, bufferLen, 0,
                          (const struct sockaddr*)(&destAddr),
                          sizeof(destAddr));
        if (sent == -1)
        { 
            fprintf(stderr, "sendto() : %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        if (sent < bufferLen)
        {
            fprintf(stderr, "not all of the message has been sent\n");
            exit(EXIT_FAILURE);
        }

        printf("%s", buffer);
        usleep(timeoutusec);        
    }

    free(buffer);
    close(sock);
    exit(EXIT_SUCCESS);
}
