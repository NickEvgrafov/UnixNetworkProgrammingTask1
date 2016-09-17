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
        printf("usage: receiver [<port>] [<timeoutms>] [SO_RCVBUF]\n");
        exit(EXIT_SUCCESS);
    }

    unsigned port = 6666;
    if (argc >= 2)
        port = atoi(argv[1]);
    printf("port = %d\n", port);

    int timeoutms = 1000;
    if (argc >= 3)
        timeoutms = atoi(argv[2]);
    printf("timeout (ms) = %d\n", timeoutms);
    useconds_t timeoutusec = timeoutms * 1000;

    int soRcvBuff = 128;
    if (argc >= 4)
    {
        soRcvBuff = atoi(argv[3]);
        soRcvBuff = soRcvBuff > 128 ? soRcvBuff : 128; 
    }
    printf("soRcvBuff = %d\n", soRcvBuff);

    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        fprintf(stderr, "socket(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int recvbuff = 0;
    int optlen = sizeof(recvbuff);
    int res = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuff, &optlen);
    if (res == -1)
    {
        fprintf(stderr, "getsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("initial recv buffer size = %d\n",recvbuff);
    }

    res = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &soRcvBuff, sizeof(soRcvBuff));
    if (res == -1)
    {
        fprintf(stderr, "setsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("called setsockopt(... %d ...)\n", soRcvBuff);
    }

    res = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvbuff, &optlen);
    if (res == -1)
    {
        fprintf(stderr, "getsockopt() : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("modified recv buffer size = %d\n",recvbuff);
    }
    
    struct sockaddr_in destAddr;
    bzero(&destAddr, sizeof(struct sockaddr_in));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = INADDR_ANY;

    int bindRes = bind(sock, (const struct sockaddr*)&destAddr, sizeof(destAddr));
    
    char buffer[65536];
    int received = 0;

    struct sockaddr_in from;
    int fromLen = sizeof(struct sockaddr_in);
    
    while (1)
    {
        received = recvfrom(sock, buffer, 65535, 0, (struct sockaddr*)&from, &fromLen);
        if (received == -1)
        {
            fprintf(stderr, "recv(): %s\n", strerror(errno));
            exit(EXIT_FAILURE);                    
        }

        char addrstr[32];
        inet_ntop(AF_INET, &from.sin_addr.s_addr, addrstr, sizeof(addrstr));

        buffer[received] = '\0';
        printf("received %d bytes from:%s\n", received, addrstr);
        printf("%s", buffer);

        usleep(timeoutusec);        
    }
    
    close(sock);

    exit(EXIT_SUCCESS);
}
