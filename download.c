#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

int connectServer(char* host, int port){
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *h;
    h = gethostbyname(host);
    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) h->h_addr)));    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }
    return sockfd;
}

void parseURL(char* url, char* host, int* port, char* path, char* user, char* pass){
    char* base = url;
    if(strncmp(base, "ftp://", 6) == 0){
        base += 6;
    } else{
        printf("Invalid URL\n");
        return;
    }
    char* barra = strchr(base, '/');
    char* arroba = strchr(base, '@');
    if(arroba != NULL && (barra == NULL || arroba < barra)){
        // user:pass@host:port/path
        printf("1\n");
        char* t = strchr(base, ':');
        memcpy(user, base, t - base);
        user[t - base] = 0;
        memcpy(pass, t + 1, arroba - t - 1);
        pass[arroba - t - 1] = 0;
        base = arroba + 1;
    }
    char* dois_pontos = strchr(base, ':');
    if(dois_pontos != NULL && (barra == NULL || dois_pontos < barra)){
        printf("2\n");
        // host:port/path
        memcpy(host, base, dois_pontos - base);
        host[dois_pontos - base] = 0;
        *port = atoi(dois_pontos + 1);
        if(barra == NULL){
            path[0] = '/';
            path[1] = 0;
        }else{
            strcpy(path, barra);
        }
    }else{
        printf("3\n");
        // host/path
        if(barra == NULL){
            strcpy(host, base);
            path[0] = '/';
            path[1] = 0;
        }else{
            memcpy(host, base, barra - base);
            host[barra - base] = 0;
            strcpy(path, barra);
        }
        *port = 21;
    }
}

void readResponseFromServer(int sockfd){
    char buf[1024];
    int bytes;

    while((bytes = read(sockfd, buf, 1023)) > 0){
        buf[bytes] = '\0';
        printf("%s", buf);
    }
}

void sendMessageToServer(int sockfd, char* message){
    int bytes;

    bytes = write(sockfd, message, strlen(message));
    if (bytes > 0)
        printf("Sent message |%s| to the server! Bytes written: %d", message, bytes);
}

int main(int argc, char **argv) {
    int sockfd, port;
    char host[256];
    char path[256];
    char user[256];
    char pass[256];
    char message[256];
    char url[] = "ftp://anonymous:ola@ftp.up.pt/pub/kodi/robots.txt";
    parseURL(url, host, &port, path, user, pass);
    printf("host: %s\nport: %d\npath: %s\nuser: %s\npass: %s\n\n", host, port, path, user, pass);
    sockfd = connectServer(host, port);
    readResponseFromServer(sockfd);
    while(1){
        printf("Enter message to send to the server: ");
        fgets(message, 256, stdin);
        if (strcmp(message, "exit") == 0)
            break;
        printf("\n");
        sendMessageToServer(sockfd, message);
        readResponseFromServer(sockfd);
    }
    return 0;
}

void downloadFile_FTP(char *url){

}