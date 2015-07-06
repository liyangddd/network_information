/*  
* server端  
* 1、client端输入"exit"，server处理请求，返回client端"exit"信息，client端关闭连接；  
* 2、client端输入地名，如"杭州"，server端处理请求，如果存在"杭州.cache文件"，则读取cache文件中的数据，返回client端；  
* 3、client端输入地名，如"杭州"，server端处理请求，如果不存在"杭州.cache文件"，则post http请求，从www.weather.com.cn网站获取杭州的天气情况，返回client端，并写入"杭州.cache"文件；  
* 4、client端输入"clear"，server端处理请求，删除掉所有cache文件；  
* 5、server端有限支持多client端并发数据请求。  
*/  
  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <ctype.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <sys/stat.h>
#include <signal.h>  
#include <errno.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include "cJSON.h"  
  
#define MAX 1024  
#define INET_ADDRSTRLEN 16  
#define DESTPORT 8090  
  
/*  
* request_http  
* description: 发送http请求，并接受返回数据  
* param: str: URL末尾拼接字符串; URL:固定地址; DestIp: web服务器IP; DestPort: http请求端口  
* return: 返回http应答数据  
*/  
char *request_http(char * str, char * URL, char * DestIp, int DestPort)  
{  
    printf("METHOD-request_http: try to request the json data...\n");  
    // http request header  
    int headlen;  
    char *head = (char *)malloc(MAX * sizeof(char));  
    strcat(head, "GET ");  
    strcat(head, URL);  
    strcat(head, str);  
    strcat(head, " HTTP/1.1\r\n");  
    strcat(head, "Host: ");  
    strcat(head, DestIp);  
    strcat(head, "\r\nConnection: keep-alive\r\n");  
    strcat(head, "Accept: */*\r\n");  
    strcat(head, "Referer: http://www.weather.com.cn/weather/101210101.shtm\r\n\r\n");  
    headlen = strlen(head);  
    //printf(head);  
  
    // create socket  
    int sockfd;  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  
    if(sockfd < 0)  
    {  
        printf("METHOD-request_http: create socket error %s(errno: %d)\n",strerror(errno),errno);  
        exit(1);  
    }  
    else  
    {  
        printf("METHOD-request_http: create socket success...\n");  
    }  
  
    struct sockaddr_in dest_addr;  
    dest_addr.sin_family = AF_INET;  
    dest_addr.sin_port = htons(DestPort);  
    dest_addr.sin_addr.s_addr = inet_addr(DestIp);  
  
    // connect  
    int ret;  
    ret = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr))    ;  
    if(ret < 0)  
    {  
        printf("METHOD-request_http: connect error %s(errno: %d)\n",strerror(errno),errno);  
        exit(1);  
    }  
    else  
    {  
        printf("METHOD-request_http: connect socket success...\n");  
    }  
  
    // send  
    ret = write(sockfd, head, headlen);  
    if(ret < 0)  
    {  
        printf("METHOD-request_http: write error %s(errno: %d)\n",strerror(errno),errno);  
        exit(1);  
    }  
    else  
    {  
        printf("METHOD-request_http: write success...\n");  
    }  
  
    // read  
    char *strResponse = (char *)malloc(MAX);  
    int flag = 1;  
    while(1)  
    {  
        ssize_t n;  
        n = read(sockfd, strResponse, MAX);  
        //if(n <= 0)  
        //{  
        //  break;  
        //}  
        strResponse[n]='\0';  
        if(flag)  
        {  
            break;  
        }  
    }  
  
    // close  
    close(sockfd);  
    printf("METHOD-request_http: return the result...\n");  
    return strResponse;  
}  
  
/*  
* extract_city_id  
* description: 抽取字符串中的城市编码信息  
* param: src: 包含城市编码信息的字符串  
* return: 返回城市编码字符串  
*/  
char *extract_city_id(char *src)  
{  
    printf("METHOS-extract_city_id: try to extract cityid...\n");  
    int i;  
    char *temp = src;  
    char *dst;  
    dst = strsep(&temp, "~");  
    for(i=0; i<3; i++)  
    {  
        strsep(&dst, "\"");  
    }  
    printf("METHOD-extract_city_id: return the result...\n");  
    return dst;  
}  
  
/*  
* extract_weather_json  
* description: 抽取字符串中的天气信息  
* param: src: 包含天气信息的字符串  
* return: 返回天气信息，json格式  
*/  
char *extract_weather_json(char *src)  
{  
    printf("METHOD-extract_weather_json: try to extract weather json...\n");  
    int i;  
    char *p = src;  
    char *dst;  
    for(i=0; i<8; i++)  
    {  
        strsep(&p, "\n");  
    }  
    dst = strsep(&p, "\n");  
    printf("METHOD-extract_weather_json: return the result...\n");  
    return dst;  
}  
  
/*  
* parse_json  
* description: 解析json数据  
* param: src: json数据字符串  
* return: 返回指定格式的数据字符串  
*/  
char *parse_json(char *src)  
{  
    printf("METHOD-parse_json: try to parse the json data...\n");  
    char *city, *cityid, *temp1, *temp2, *weather;  
    char *result = (char*)malloc(1024);  
    cJSON *root, *format;  
    root = cJSON_Parse(src);  
    format = cJSON_GetObjectItem(root,"weatherinfo");  
    city = cJSON_GetObjectItem(format,"city")->valuestring;  
    cityid = cJSON_GetObjectItem(format,"cityid")->valuestring;  
    temp1 = cJSON_GetObjectItem(format,"temp1")->valuestring;  
    temp2 = cJSON_GetObjectItem(format,"temp2")->valuestring;  
    weather = cJSON_GetObjectItem(format,"weather")->valuestring;  
  
    strcat(city, ",");  
    strcat(city, cityid);  
    strcat(city, ",");  
    strcat(city, "high temprature:");  
    strcat(city, temp1);  
    strcat(city, ",");  
    strcat(city, "low temprature:");  
    strcat(city, temp2);  
    strcat(city, ",");  
    strcat(city, weather);  
    strcpy(result, city);  
    printf("METHOD-parse_json: return the result...\n");  
    return result;  
}  
  
/*  
* write_cache  
* description: 将字符串数据写入cache文件  
* param: src: 写入字符串; name: 写入文件名; count: 写入长度  
* return: none  
*/  
void write_cache(char *src, char *name, size_t count)  
{  
    printf("METHOD-write_cache: try to write weather info to cache file...\n");  
    int fd;  
    fd = open(name, O_RDWR|O_CREAT, S_IRWXU);  
    if(fd < 0)  
    {  
        printf("METHOD-write_cache: open text file failed!\n");  
        exit(1);  
    }  
    ssize_t ret;  
    ret = write(fd, src, count);  
    if(ret < 0)  
    {  
        printf("METHOD-write_cache: write to text file failed!\n");   
        exit(1);  
    }  
    printf("METHOD-write_cache: success to write weather info to cache file...\n");  
    close(fd);  
}  
  
/*  
* read_cache  
* description: 读取cache文件内容  
* param: name: 读取文件名  
* return: 返回cache文件中的字符串  
*/  
char *read_cache(char *name)  
{  
    printf("METHOD-write_cache: try to read weather info from cache file...\n");  
    int fd;  
    fd = open(name, O_RDWR);  
    if(fd < 0)  
    {  
         printf("METHOD-read_cache: open text file failed!\n");  
         return NULL;  
    }  
    ssize_t ret;  
    char *dst = (char *)malloc(1024);  
    ret = read(fd, dst, 1024);  
    if(ret < 0)  
    {  
        printf("METHOD-read_cache: read text file failed!\n");  
        return NULL;  
    }  
    printf("METHOD-write_cache: return the result...\n");  
    printf("%s\n", dst);  
    return dst;  
}  
  
int main()  
{  
    // 测试request_http  
    // printf("%s\n", request_http("101030100.html", "/data/cityinfo/", "61.4.185.201", 80));  
      
    // 测试extract_city_id  
    // char *temp;  
    // char *result;  
    // temp = request_http("杭州","/search?cityname=","61.4.185.213",80);  
    // result = extract_city_id(temp);  
    // printf("%s\n", result);  
      
    // 测试extract_weather_json  
    // char *temp;  
    // char *result;  
    // temp = request_http("101030100.html", "/data/cityinfo/", "61.4.185.201", 80);  
    // result = extract_weather_json(temp);  
    // printf("%s\n", result);  
      
    // 测试parse_json  
    // char *temp;  
    // char *result;  
    // char *weather;  
    // temp = request_http("101030100.html", "/data/cityinfo/", "61.4.185.201", 80);  
    // result = extract_weather_json(temp);  
    // weather = parse_json(result);  
    // printf("%s\n", weather);  
      
    // 测试write_cache  
    // char *str = "hello, world!";  
    // write_cache(str, "hello", 16);  
  
    int sock;    // listen socket  
    int cock;    // connect socket  
    int len = sizeof(struct sockaddr_in);  
    char send_data[MAX];  
    char recv_data[MAX];  
    char addr_p[INET_ADDRSTRLEN];  
    struct sockaddr_in s_addr;  
    struct sockaddr_in c_addr;  
    memset(&s_addr, 0, sizeof(s_addr));  
    memset(&c_addr, 0, sizeof(c_addr));  
    pid_t pid;  
  
    signal(SIGCHLD, SIG_IGN);     // 忽略产生僵尸进程  
  
    // set addr info  
    s_addr.sin_family = AF_INET;  
    // s_addr.sin_addr.s_addr = INADDR_ANY;  
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    s_addr.sin_port = htons(DESTPORT);  
    printf("success to set addr info...\n");  
  
    // create socket  
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
        perror("fail to create socket...\n");  
        exit(1);  
    }  
    printf("success to create socket...\n");  
  
    // bind  
    if(bind(sock, (struct sockaddr *)&s_addr, sizeof(s_addr)) == -1)  
    {  
        perror("fail to bind...\n");  
        exit(1);  
    }  
    printf("success to bind...\n");  
  
    // listen  
    if(listen(sock, 10) == -1)  
    {  
        perror("fail to listen...\n");  
        exit(1);  
    }  
    printf("success to listen...\n");  
  
    printf("waiting for connecting...\n");  
  
    while(1)  
    {  
        // accept, block here  
        if((cock = accept(sock, (struct sockaddr *)&c_addr, &len)) == -1)  
        {  
            perror("fail to accept...\n");  
            exit(1);  
        }  
        printf("----------------------------------\n");  
        printf("connection established...\n");  
        printf("----------------------------------\n");  
  
        inet_ntop(AF_INET, &c_addr.sin_addr, addr_p, sizeof(addr_p));  
        printf("connected ip: %s\n", addr_p);  
          
        if((pid = fork()) < 0)  
        {  
            perror("fail to fork...\n");  
            exit(1);  
        }  
        else if(pid == 0)  
        {  
            while(1)  
            {  
                memset(&send_data, 0, sizeof(send_data));  
                memset(&recv_data, 0, sizeof(recv_data));  
  
                // read  
                int flag;  
                if((flag = read(cock, recv_data, MAX)) == -1)  
                {  
                    perror("fail to read...\n");  
                    exit(1);  
                }  
                else if(flag == 0)  
                {  
                    printf("client closed abnormally...\n");  
                    exit(0);  
                }  
  
                char *recv_data1 = (char *)malloc(MAX);  
                strcpy(recv_data1, recv_data);  
  
                printf("recv msg from client %s: %s\n", addr_p, recv_data);  
              
                // if client apply to close the connection  
                if(strcmp(recv_data, "exit") == 0)  
                {  
                    printf("client %s apply to break the connection...\n", addr_p);  
                    if(write(cock, recv_data, MAX) == -1)  
                    {  
                        perror("fail to write...\n");  
                        exit(1);  
                    }  
                    exit(0);  
                }  
                else if(strcmp(recv_data, "clear") == 0)  
                {  
                    system("rm /home/sl/learn/*.cache");  
                    strcat(send_data, "msg has been recved by server.\n");  
                    strcat(send_data, "server try to clean cache.\n");  
                    printf("server try to delete the cache file...\n");  
                }  
                else  
                {  
                    int exist;  
                    char *file_name = recv_data1;  
                    file_name = strcat(file_name, ".cache");  
                    printf("%s\n", recv_data);  
                    exist = access(file_name, 0);  
                    if(exist == 0)  
                    {  
                        char *cache = read_cache(file_name);  
                        strcat(send_data, "msg has been recved by server.\n");  
                        strcat(send_data, "server try to offer the weather info.\n");  
                        strcat(send_data, cache);  
                        strcat(send_data, "<------ CACHE\n");  
                    }  
                    else  
                    {  
                        char *cityid;  
                        printf("------------------------------------------\n");  
                        printf("%s\n", "child process try to request cityinfo...");  
                        printf("------------------------------------------\n");  
                        char *city_id_json = request_http(recv_data,"/search?cityname=","61.4.185.213",80);  
                        printf("%s\n", city_id_json);  
                        printf("------------------------------------------------------\n");  
                        printf("%s\n", "child process try to extract cityid from cityinfo...");  
                        printf("------------------------------------------------------\n");  
                        cityid = extract_city_id(city_id_json);  
                        char *ss = strcat(cityid, ".html");  
                        printf("------------------------------------------\n");  
                        printf("%s\n", "child process try to request weatherinfo");  
                        printf("------------------------------------------\n");  
                        char *city_weather_info = request_http(ss, "/data/cityinfo/", "61.4.185.201", 80);  
                        printf("%s\n", city_weather_info);  
                        printf("--------------------------------------------------------------\n");  
                        printf("%s\n", "child process try to extract weatherjson from weatherinfo...");  
                        printf("--------------------------------------------------------------\n");  
                        char *city_weather_json = extract_weather_json(city_weather_info);  
                        printf("%s\n", city_weather_json);  
                        printf("-----------------------------------------------\n");  
                        printf("%s\n", "child process try to parse the weatherjson...");  
                        printf("-----------------------------------------------\n");  
                        char *weather = parse_json(city_weather_json);  
                        printf("%s\n", weather);  
                        ssize_t mm = strlen(weather);  
                        write_cache(weather, file_name, mm);  
                        strcat(send_data, "msg has been recved by server.\n");  
                        strcat(send_data, "server try to offer the weather info.\n");  
                        strcpy(send_data, weather);  
                    }  
                }  
  
                if(write(cock, send_data, MAX) == -1)  
                {  
                    perror("fail to write...\n");  
                    exit(1);  
                }  
            }  
        }  
    }  
    close(sock);  
  
    return 0;  
}  
