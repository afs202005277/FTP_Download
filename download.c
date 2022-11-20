#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>

#define DEFAULT_PORT 21
#define INPUT_BUF_SIZE 1024
#define DEFAULT_BUF_SIZE 256

struct ConnectionParams
{
    int port, sockfd;
    char host[DEFAULT_BUF_SIZE];
    char path[DEFAULT_BUF_SIZE];
    char user[DEFAULT_BUF_SIZE];
    char pass[DEFAULT_BUF_SIZE];
    char ip[DEFAULT_BUF_SIZE];
};

struct ConnectionParams readResponseFromServer(int sockfd)
{
    char *buf = malloc(INPUT_BUF_SIZE);
    struct ConnectionParams params;
    size_t bytes = 0;
    FILE *file = fdopen(sockfd, "r");
    while (getline(&buf, &bytes, file) != -1)
    {
        printf("%s", buf);
        if (strstr(buf, "-") == NULL)
        {
            break;
        }
    }
    if (memcmp(buf, "227", 3) == 0)
    {
        char *start = strstr(buf, "(")+1;
        char *token = strtok(start, ",");
        int i = 0;
        params.host[0] = '\0';
        while (token != NULL)
        {
            if (i < 4)
            {
                strcat(params.ip, token);
                if (i < 3)
                {
                    strcat(params.ip, ".");
                }
            }
            else if (i == 4)
            {
                params.port = atoi(token) * 256;
            }
            else if (i == 5)
            {
                params.port += atoi(token);
            }
            token = strtok(NULL, ",");
            i++;
        }
    }
    free(buf);
    return params;
}

void connectServer(struct ConnectionParams *params)
{
    struct sockaddr_in server_addr;
    struct hostent *h;
    char *ip;
    if (params->host[0] == '\0')
    {
        ip = params->ip;
    }
    else
    {
        h = gethostbyname(params->host);
        ip = inet_ntoa(*((struct in_addr *)h->h_addr));
    }

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(params->port);  /*server TCP port must be network byte ordered */

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
        printf("ip: %s\n", ip);
        printf("%d\n", params->port);
        exit(-1);
    }
    if (params->host[0] != '\0')
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
        params->port = DEFAULT_PORT;
    }
}

void sendMessageToServer(int sockfd, char *message)
{
    int bytes;

    bytes = write(sockfd, message, strlen(message));
    if (bytes < 0)
    {
        perror("write()");
        exit(-1);
    }
}

void login(struct ConnectionParams connectionParams)
{
    char buf[DEFAULT_BUF_SIZE];
    strcpy(buf, "user ");
    strcat(buf, connectionParams.user);
    strcat(buf, "\r\n");

    sendMessageToServer(connectionParams.sockfd, buf);
    readResponseFromServer(connectionParams.sockfd);

    strcpy(buf, "pass ");
    strcat(buf, connectionParams.pass);
    strcat(buf, "\r\n");
    sendMessageToServer(connectionParams.sockfd, buf);
    readResponseFromServer(connectionParams.sockfd);
}

void downloadFile(int sockfd, char* path){
    char* buf = malloc(INPUT_BUF_SIZE);
    char fileName[DEFAULT_BUF_SIZE];
    size_t bytes;
    for (int i = strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '/')
        {
            strcpy(fileName, path + i + 1);
            break;
        }
    }
    FILE *downloadedFile = fopen(fileName, "w");
    FILE *received = fdopen(sockfd, "r");
    while (getline(&buf, &bytes, received) != -1)
    {
        fprintf(downloadedFile, "%s", buf);
    }
    free(buf);
}

void setUpPassive(struct ConnectionParams *downloadFileParams)
{
    char buf[DEFAULT_BUF_SIZE];
    strcpy(buf, "pasv\r\n");
    sendMessageToServer(downloadFileParams->sockfd, buf);

    struct ConnectionParams tmp = readResponseFromServer(downloadFileParams->sockfd);

    connectServer(&tmp);

    strcpy(buf, "retr ");
    strcat(buf, downloadFileParams->path);
    strcat(buf, "\r\n");
    sendMessageToServer(downloadFileParams->sockfd, buf);
    readResponseFromServer(downloadFileParams->sockfd);

    downloadFile(tmp.sockfd, downloadFileParams->path);
}

int main(int argc, char **argv)
{
    char url[] = "ftp://anonymous:ola@ftp.up.pt:21/pub/kodi/robots.txt";
    struct ConnectionParams connectionParams;
    connectionParams.user[0] = '\0';
    connectionParams.pass[0] = '\0';
    parseURL(&connectionParams, url);
    if (connectionParams.user[0] == '\0'){
        strcpy(connectionParams.user, "anonymous");
        strcpy(connectionParams.pass, "qualquer");
    }
    printf("host: %s\nport: %d\npath: %s\nuser: %s\npass: %s\n\n", connectionParams.host, connectionParams.port, connectionParams.path, connectionParams.user, connectionParams.pass);
    connectServer(&connectionParams);
    login(connectionParams);
    setUpPassive(&connectionParams);
    return 0;
}