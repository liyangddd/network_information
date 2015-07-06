/*  
* client端  
* 1、发送"城市名", "exit", "clear"请求；  
* 2、接受server端返回的处理信息并打印；  
* 3、保持与server端的连接状态，可重复发送请求，直至发送"exit"断开连接，或者异常关闭连接。  
*/  
  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <ctype.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
  
#define MAX 1024  
#define DESTIP "127.0.0.1"  
#define DESTPORT 8090  
  
int main()  
{  
    int cock;  
    char send_data[MAX];  
    char recv_data[MAX];  
    struct sockaddr_in s_addr;  
    memset(&s_addr, 0, sizeof(s_addr));  
  
    // set server addr info  
    s_addr.sin_family = AF_INET;  
    inet_pton(AF_INET, DESTIP, &s_addr.sin_addr);  
    s_addr.sin_port = htons(DESTPORT);  
    printf("success to set server addr info...\n");  
      
    // create socket  
    if((cock = socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
        perror("fail to create socket...\n");  
        exit(1);  
    }  
    printf("success to create socket...\n");  
  
    // connect  
    if(connect(cock, (struct sockaddr*)&s_addr, sizeof(s_addr)) == -1)  
    {  
        perror("fail to connect...\n");  
        exit(1);  
    }  
    printf("success to connect...\n");  
  
    while(1)  
    {  
        memset(&send_data, 0, sizeof(send_data));  
        memset(&recv_data, 0, sizeof(send_data));  
        printf("please input the msg: ");  
        fgets(send_data, 1024, stdin);  
        send_data[strlen(send_data)-1] = '\0';  
  
        // write  
        if(write(cock, send_data, strlen(send_data)) == -1)  
        {  
            perror("fail to write...\n");  
            exit(1);  
        }  
        printf("send msg to server: %s\n", send_data);  
          
        // read  
        if(read(cock, recv_data, MAX) == -1)  
        {  
            perror("fail to read...\n");  
            exit(1);  
        }  
          
        if(strcmp(recv_data, "exit") == 0)  
        {  
            printf("server response to close the connection...\n");  
            close(cock);  
            exit(0);  
        }  
        printf("response msg from server: %s\n", recv_data);  
    }  
  
    return 0;  
}  
