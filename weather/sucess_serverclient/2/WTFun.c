#include "head.h"
#include "printmsg.h"

#define URL "www.wuhantianqi114.com"
//#define URL "http://m.weather.com.cn/data/101010100.html"
#define PAGE "/"
#define PORT 80
#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebkit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.114 Safari/537.36"
#define ACCEPTLANGUAGE "zh-CN,zh;q=0.8,en;q=0.6,en-US;q=0.4,en-GB;q=0.2"
#define ACCEPTENCODING "gzip,deflate,sdch"

//程序名：WTFun（weather report for fun）
//代码量：180行（有效）
//作者：
//时间：2014/01/20
//感言：因为我也是刚刚开始学linux下c编程，代码不是十分的规范
//	也没有遵守什么命名规则，完全是自己胡写的，所以还请读到
//	了代码的人见谅

void recv_message(int info[2]);

int main(int argc,char* argv[])
{
	//本段代码为socket编程的基本内容，不熟悉的朋友可以查找有关书籍或博客
	struct sockaddr_in remote;
	int sock;
	if( (sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0)
	{
		printf("can't create TCP socket\n");
		exit(1);
	}

	struct hostent *hent;
	//通过域名得到ip
	if( (hent = gethostbyname(URL)) == NULL)
	{
		printf("can't get ip\n");
		exit(1);     
	}

	char ip[20] = {0};
	if(inet_ntop(AF_INET,(void*)hent->h_addr_list[0],ip,20) == NULL)
	{
		printf("can't resolve host\n");
		exit(1);
	}

	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);
	remote.sin_addr.s_addr = inet_addr(ip);
	
	//建立链接
	if(connect(sock,(struct sockaddr*)&remote,sizeof(struct sockaddr)) < 0)
	{
		printf("could not connect\n");
		exit(1);
	}

	//初始化http请求包的内容
	char message[512] = {0};
	char *tpl="GET %s HTTP/1.1\r\nHost:%s\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUser-Agent:%s\r\nAccept-Language:%s\r\n\r\n";
	sprintf(message,tpl,PAGE,URL,USERAGENT,ACCEPTLANGUAGE);

	//发送http请求包
	int sent = 0;
	int tmpres;
	while(sent<strlen(message))
	{
		tmpres = send(sock,message+sent,strlen(message)-sent,0);
		if(tmpres == -1)
		{
			printf("can't send query\n");
			exit(1);
		}
		sent += tmpres;
	}


	//open a file which will keep messages receive from URL
	int fd;
	fd = open("weather.txt",O_RDWR|O_CREAT|O_TRUNC,0666);
	if(fd == -1)
	{
		printf("open file fail\n");
		exit(1);
	}

	int info[2];
	info[0] = sock;
	info[1] = fd;
	//想实验一下多线程的知识，于是把recv放到了一个线程中
	pthread_t thread;
	pthread_create(&thread,NULL,(void*)(&recv_message),(void*)info);
	sleep(1);
	print_weather(fd);
	close(fd);
	close(sock);
	return 0;
}


void recv_message(int info[2])
{
	char buf[1024] = {0};
	int htmlstart = 0;
	char *htmlcontent;
	int res;
	int sock = info[0];
	int fd = info[1];
	while( (res = recv(sock,buf,1023,0)) >0 )
	{
		if(htmlstart ==0 )
		{
			htmlcontent = strstr(buf,"\r\n\r\n");
			if(htmlcontent != NULL)
			{
				htmlstart = 1;
				htmlcontent += 4;
			}
		}
		else
		{
			htmlcontent = buf;
		}

		if(htmlstart)
		{
			if(write(fd,htmlcontent,strlen(htmlcontent)) != strlen(htmlcontent))
			{
				printf("write file fail\n");
				return;
			}
		}
		memset(buf,0,res);
	}
	if(res < 0)
		printf("receive date fail\n");

}
