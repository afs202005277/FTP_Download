#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

struct ConnectionParams
{
    int port, sockfd;
    char host[256];
    char path[256];
    char user[256];
    char pass[256];
};

void readResponseFromServer(int sockfd)
{
    char buf[1024];
    FILE *file = fdopen(sockfd, "r");
    while ((NULL != fgets(buf, sizeof(buf), file)) > 0)
    {
        printf("%s", buf);
    }
    fclose(file);
}

void connectServer(struct ConnectionParams *params)
{
    struct sockaddr_in server_addr;
    struct hostent *h;
    h = gethostbyname(params->host);

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr))); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(params->port);                                         /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((params->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    if (connect(params->sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }
    readResponseFromServer(params->sockfd);
}

void parseURL(struct ConnectionParams *params, char *url)
{
    char *base = url;
    if (strncmp(base, "ftp://", 6) == 0)
    {
        base += 6;
    }
    else
    {
        printf("Invalid URL\n");
        return;
    }
    char *barra = strchr(base, '/');
    char *arroba = strchr(base, '@');
    if (arroba != NULL && (barra == NULL || arroba < barra))
    {
        // user:pass@host:port/path
        char *t = strchr(base, ':');
        memcpy(params->user, base, t - base);
        params->user[t - base] = 0;
        memcpy(params->pass, t + 1, arroba - t - 1);
        params->pass[arroba - t - 1] = 0;
        base = arroba + 1;
    }
    char *dois_pontos = strchr(base, ':');
    if (dois_pontos != NULL && (barra == NULL || dois_pontos < barra))
    {
        // host:port/path
        memcpy(params->host, base, dois_pontos - base);
        params->host[dois_pontos - base] = 0;
        params->port = atoi(dois_pontos + 1);
        if (barra == NULL)
        {
            params->path[0] = '/';
            params->path[1] = 0;
        }
        else
        {
            strcpy(params->path, barra);
        }
    }
    else
    {
        // host/path
        if (barra == NULL)
        {
            strcpy(params->host, base);
            params->path[0] = '/';
            params->path[1] = 0;
        }
        else
        {
            memcpy(params->host, base, barra - base);
            params->host[barra - base] = 0;
            strcpy(params->path, barra);
        }
        params->port = 21;
    }
}

void sendMessageToServer(int sockfd, char *message)
{
    int bytes;

    bytes = write(sockfd, message, strlen(message));
    if (bytes > 0)
        printf("Sent message |%s| to the server! Bytes written: %d\n", message, bytes);
    else
    {
        perror("write()");
        exit(-1);
    }
}

void login(struct ConnectionParams connectionParams)
{
    char buf[256];
    memset(buf, 0, 256);
    strcpy(buf, "user ");
    strcat(buf, connectionParams.user);
    strcat(buf, "\r\n");

    sendMessageToServer(connectionParams.sockfd, buf);
    readResponseFromServer(connectionParams.sockfd);
    printf("ola\n");
    memset(buf, 0, 256);
    strcpy(buf, "pass ");
    strcat(buf, connectionParams.pass);
    strcat(buf, "\r\n");
    sendMessageToServer(connectionParams.sockfd, buf);
    readResponseFromServer(connectionParams.sockfd);
}

int main(int argc, char **argv)
{
    char url[] = "ftp://anonymous:ola@ftp.up.pt/pub/kodi/robots.txt";
    struct ConnectionParams connectionParams;
    parseURL(&connectionParams, url);
    printf("host: %s\nport: %d\npath: %s\nuser: %s\npass: %s\n\n", connectionParams.host, connectionParams.port, connectionParams.path, connectionParams.user, connectionParams.pass);
    connectServer(&connectionParams);
    login(connectionParams);
    sendMessageToServer(connectionParams.sockfd, "pasv\r\n");
    readResponseFromServer(connectionParams.sockfd);
    return 0;
}

void downloadFile_FTP(char *url)
{
}