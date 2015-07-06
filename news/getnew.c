#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <ctype.h>  
#include <sys/socket.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>  
#include <fcntl.h>
#include <netdb.h>
#include "cJSON.h"
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <iconv.h>

#define MAX 20480
#define INET_ADDRSTRLEN 16
#define DESTPORT 80

char local_city_id[9];

ssize_t readn(int fd, void *vptr, size_t n)
{
  size_t nleft;
  ssize_t nread;
  char *ptr;
  
  ptr = vptr;
  nleft = n;
  while (nleft > 0)
  {
    if ((nread = read(fd, ptr, nleft)) < 0)
    {
      if (errno == EINTR)
	nread = 0;
      else
	return(-1);
    }else if (nread == 0)
      break;
    nleft -= nread;
    ptr += nread;
  }
  return (n - nleft);
}

char *http_request(char *str, char *URL, char *DestIP, int DestPort)
{
   //printf("Try to request the json data from weather.com\n");
   int headlen;                                     // http request header
   char *head = (char *)malloc(MAX);
   strcat(head, "GET ");
   strcat(head, URL);
   strcat(head, str);
   strcat(head, " HTTP/1.1\r\n");
   strcat(head, "Host: ");
   strcat(head, DestIP);
   strcat(head, "\r\nConnection: keep-alive\r\n");  
   strcat(head, "Accept: */*\r\n");  
   //strcat(head, "Referer:
http://www.weather.com.cn/weather/101210101.shtm\r\n\r\n");  
   //http://m.weather.com.cn/data/101010100.html   
   strcat(head, "Referer: http://m.weather.com.cn/data/101010100.html\r\n\r\n");
   headlen = strlen(head);  
   
   //create socket
   int sockfd;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
   {
	printf("http request: create socket error %s (errno: %d)\n",
strerror(errno), errno);
	exit(1);
   }
   else
   {	//printf("http request:create socket success\n"); 
   }

   struct sockaddr_in dest_addr;
   dest_addr.sin_family = AF_INET;
   dest_addr.sin_port = htons(DestPort);
   dest_addr.sin_addr.s_addr = inet_addr(DestIP);

   char *ptr, **pptr, str_addr[32];
   ptr = DestIP;
   struct hostent *hptr;
   if ((hptr = gethostbyname(DestIP)) == NULL)  
   { printf("gethostbyname erro %s.\n", DestIP);}
   pptr = hptr -> h_addr_list;
   
   dest_addr.sin_addr.s_addr = inet_addr(inet_ntop(hptr->h_addrtype, *pptr,
str_addr, sizeof(str_addr)));

   // connect
   int ret;
   ret = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
   if (ret < 0)
   {  printf("http request: connect error %s(errno: %d)\n", strerror(errno),
errno);  }
   else{//printf("http request: connect socket succes.\n");
   }

   //send
   ret = write(sockfd, head, headlen);
   if (ret < 0)
   {
	printf("http request: write error %s(errno: %d)\n", strerror(errno),
errno);
        exit(1);
   }
   else{  //printf("http request: write succes.\n");  
   }
   
   // read
   char *strResponse = (char *)malloc(MAX);
   memset(strResponse, 0, MAX);
  
   ssize_t n = readn(sockfd, strResponse, MAX);
  
   printf("read : %d\n", n);
  
   strResponse[MAX] = '\0';
   close(sockfd);
   //printf("*******************%s\n", strResponse);
   return strResponse;
}

char *extract_news_json(char *src, int i_for)
{ 
   //printf("extract weather json: try to extract weather json.\n");
   int i;
   char *p = src;
   char *dst;
   for(i = 0; i < i_for; i++)
   {
	strsep(&p, "\n");
       //printf("*********************%d %s\n",i, p);
   }
   dst = strsep(&p, ";");
   //printf("&&&&&&&&&&&&&&&&&&%s\n", dst);
   //printf("extract weather json: extract weather json success.\n");
   return dst;
}

char *parse_json(char *src)
{
   //printf("parse json: try to parse json data.\n");
   char *city, *cityid, *temp1, *temp2, *weather;
   char *result = (char *)malloc(MAX);
   cJSON *root, *format;
   root = cJSON_Parse(src);
   format = cJSON_GetObjectItem(root,"last_time") -> valuestring;  
   //city = cJSON_GetObjectItem(format,"serverSeconds")->valuestring; 
   printf("city:::::::::::%s", format);
  // strcat(city, ", ");  
   //strcat(city, cityid);  
   //strcat(city, ", "); 
   //strcat(city, weather);  
   //strcpy(result, city);  
   //printf("parsejson success.\n");  
   return result;  
}

void get_news()
{
   char *re = (char *)malloc(MAX);
   memset(re, 0, MAX);
   char *out = (char *)malloc(MAX);
   memset(out, 0, MAX);
   re = http_request("rollnews_ch_out_interface.php","/interface/",
	     "roll.news.sina.com.cn", 80);
   
   g2u(re, strlen(re), out, MAX);
   //printf("%s\n", out);
   char *new_json = extract_news_json(out, 19);
   printf("%s\n", new_json);
 
   free(re);
   free(out);
}

//int code_convert()和 int g2u()用于编码之间的转换,如 将gb2312转换为utf-8 
int code_convert(char *from_char, char *to_char, char *inbuf, int
inlen,char *outbuf, int outlen)
{
  iconv_t cd;
  int rc;
  char **pin = &inbuf;
  char **pout = &outbuf;
  
  cd = iconv_open(to_char, from_char);
  if(cd == 0) return -1;
  memset(outbuf, 0, outlen);
  if(iconv(cd, pin, &inlen, pout, &outlen) == -1) return -1;
  iconv_close(cd);
  return 0;
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
  return code_convert("gb18030", "utf-8", inbuf, inlen, outbuf, outlen);
}


int main()
{
  get_news();
}
