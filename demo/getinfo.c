/*
 *该文件用于统一之前分散写的获取天气信息，微博信息，新闻信息的程序；
 *这是一个服务器程序，与客户端通过TCPsocket进行通信；
 *客户端输入“1，城市名，定时时间”，“2，微博昵称”或“3”来分别获取天气，微博，新闻
 *writed by liyang
*/
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define MAX 1024
#define PORT 8080

//该函数返回定时的时间（单位为秒），用于定时显示天气信息；
//tm_now结构返回客户端输入的定时时间，tm_now为服务器接到客户定时时间时的当前时间
int time_sleep(int hour, int minute, int second) {
  struct tm *tm_now;
  struct tm *tm_in;
  time_t time_now, time_in, time_sleep;
  tm_in = (struct tm *)malloc(sizeof(struct tm));
  //当前时间，单位为秒
  time_now = time((time_t *)NULL);
  tm_now = gmtime(&time_now);
  printf("now is : %s", ctime(&time_now));
 
  //将客户输入时间转换为struct tm结构，便于之后求客户时间（相对于1970年）的秒数，
  tm_in->tm_sec = second;
  tm_in->tm_min = minute;
  tm_in->tm_hour = hour;
  tm_in->tm_mday = tm_now->tm_mday;
  tm_in->tm_mon = tm_now->tm_mon;
  tm_in->tm_year = tm_now->tm_year;
  tm_in->tm_wday = tm_now->tm_wday;
  tm_in->tm_yday = tm_now->tm_yday;
  tm_in->tm_isdst = tm_now->tm_isdst;
  //求客户时间（相对于1970年）的秒数
  time_in = mktime(tm_in);
  printf(" in is : %s", ctime(&time_in));
  //求当前时间与客户时间的差，若客户时间小于当前时间则默认客户的时间（闹钟）为次日到时
  time_sleep = difftime(time_in, time_now);
  time_sleep = time_sleep >= 0 ? time_sleep : (time_sleep + 24*60*60);
  printf("sleep time: %d\n", (int)time_sleep);
  free(tm_in);
  return time_sleep;
}

int main(int argc, char *argv[]) {
  char *recv_buf = (char *)malloc(MAX);
  char addr_buf[MAX];
  socklen_t len;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(PORT);
  int client_socket;
  
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Failed to create socket\n");
    exit(1);
  }
  if (bind(server_socket, (struct sockaddr *)&server_addr,
    sizeof(server_addr)) == -1) {
    perror("Failed to bind\n");
    exit(2);
  }
  if (listen(server_socket, 10) == -1) {
    perror("Failed to listen\n");
    exit(3);
  }
  printf("waiting...\n");
  
  //通过socket获取客户的请求
  while (1) {
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
&len)) == -1) {
      perror("Failed to accept\n");
      exit(4);
    }
    printf("Client %s connencting by %d port.\n",
inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
    //创建子进程来处理客户的请求
    pid_t pid;
    pid = fork();
    switch (pid) {
      case -1:
	perror("Fork failed\n");
	exit(6);
      case 0:
      {
	int length = recv(client_socket, recv_buf, MAX, 0);
        if (length < 0) {
          perror("Failed to recv\n");
          exit(5);
	}
	char *client_str = recv_buf;
	char *client_command = strsep(&client_str, ",");
	//client_command返回客户的请求“1”，“2”或“3”
        printf("client command is %s\n", client_command);
	//客户请求天气信息的处理（client_command = 1）
	if (strcmp(client_command, "1") == 0) {
	  char *client_str23 = client_str; //client_str = "beijing,10:20:01"
	  char *city_name = strsep(&client_str23, ","); //city_name = "beijing"
	  char* client_time = client_str23; //client_str3 ="10:20:01"
	  printf("city_name %s , %s\n", city_name, client_time);
	  char *client_time_hour = strsep(&client_time, ":"); //hour = 10
          char *client_time_min = strsep(&client_time, ":"); //minute = 20
          char *client_time_sec = client_time;     //second = 01
          int tm_sleep =
time_sleep(atoi(client_time_hour),atoi(client_time_min), atoi(client_time_sec));
	  char* com = (char *)malloc(30); 
	  strcpy(com, "./weather ");
	  strcat(com, city_name);
	  //利用函数time_sleep()的返回值来为客户定时显示天气信息
          usleep(1000000*tm_sleep);
	  system(com);
	  free(com);
	}
	//客户请求微博信息的处理（client_command = 2）
	if (strcmp(client_command, "2") == 0) {
	  char *nickname = client_str;
	  printf("nickname: %s\n", nickname);
	  char *com = (char *)malloc(30);
	  strcpy(com, "./send_weibo.py ");
	  strcat(com, nickname);
	  system(com);
	  free(com);
	}
	//客户请求新闻信息的处理（client_command = 3）
	if (strcmp(client_command, "3") == 0) {
	  //system("python GetNews.py");
	  execlp("/home/yang/C++/test_client/GetNews.py","GetNews.py",(char*)0);
	}
	break;
      }
    }
    
  }
  exit(0);
}