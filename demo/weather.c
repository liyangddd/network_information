/*
 *该文件用于从网络接口（“中国天气”接口http://m.weather.com.cn/data/城市id.html
 *      和www.weather.com.cn/data/cityinfo/城市id.html）获取
 *      中国所有地区的天气情况并返回结果到控制台和文件“***.cache”中。
 *编译环境：ubuntu
 *编译：gcc -lm cJSON.c weather.c -o weather
 *     进行编译时要加入-lm参数，因为其依赖的文件“cJson.c”中使用了数学库math.h，
 *     若不加-lm参数,编译weather.c将会出错。
 *运行：./weather city_name
 *Code by liyang.
*/
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

//http request,该函数用socket来get得到网站返回的内容
char *http_request(char *str, char *URL, char *DestIP, int DestPort)
{
   //printf("Try to request the json data from weather.com\n");
   int headlen;                                     // http request header
   char *head = (char *)malloc(MAX);
   if (head == NULL)
     printf("malloc to head failed.\n");
   strcat(head, "GET ");
   strcat(head, URL);
   strcat(head, str);
   strcat(head, " HTTP/1.1\r\n");
   strcat(head, "Host: ");
   strcat(head, DestIP);
   strcat(head, "\r\nConnection: keep-alive\r\n");  
   strcat(head, "Accept: */*\r\n");  
   //strcat(head, "Referer: http://www.weather.com.cn/weather/101210101.shtm\r\n\r\n");  
   //http://m.weather.com.cn/data/101010100.html   
   strcat(head, "Referer: http://m.weather.com.cn/data/101010100.html\r\n\r\n");
   headlen = strlen(head);  
   
   //create socket
   int sockfd;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
   {
	printf("http request: create socket error %s (errno: %d)\n", strerror(errno), errno);
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
   //printf("****************%s\n", inet_ntop(hptr->h_addrtype, *pptr, str_addr, sizeof(str_addr)));
   dest_addr.sin_addr.s_addr = inet_addr(inet_ntop(hptr->h_addrtype, *pptr, str_addr, sizeof(str_addr)));

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
	printf("http request: write error %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
   }
   else{  //printf("http request: write succes.\n");  
   }
   
   // read
   char *strResponse = (char *)malloc(MAX);
   if (strResponse == NULL)
     printf("malloc to strRespose failed.\n");
   memset(strResponse, 0, MAX);
  
   ssize_t n = read(sockfd, strResponse, MAX);
  
   //printf("read : %d\n", n);
  
   strResponse[MAX] = '\0';
   close(sockfd);
   //printf("*******************%s\n", strResponse);
   return strResponse;
}

// extract city ID
char *extract_city_id(char *src)
{
   //printf("extract city ID:\n");
   int i;
   char *temp = src;
   char *dst = strsep(&temp, "~");
   //后期修改加入了该行，从而可以在用户输入城市名错误时也可以得到结果
   strsep(&dst, "{");
   for (i = 0; i < 3; i++)
   {
	strsep(&dst, "\"");
   }
   if (!dst) {
     perror("the city name input is error.\n");
     exit(0);
   }
   //这句是对输入城市名为“北京”时的特殊处理，
   //因为输入“北京”可能返回的城市id为北京的某个地方id"10101010000A"，
   //这里将id截取为默认的北京id“101010100”
   dst[9] = '\0';
   return dst;
}


// extract weather json, 
char *extract_weather_json(char *src, int i_for)
{ 
   //printf("extract weather json: try to extract weather json.\n");
   int i;
   char *p = src;
   char *dst;
   
   strsep(&p, " ");
   char *flag_find = strsep(&p, " ");
   //printf("wwwwwwwww: %s\n", flag_find);
   if (strcmp(flag_find, "200") == 0) {
     for(i = 0; i < i_for; i++)
     {
	  strsep(&p, "\n");
	//printf("*********************%d %s\n",i, p);
     }
     dst = strsep(&p, "\n");
   }
   else {
     printf("the city name input is error.\n");
     exit(1);
   }
   
   //printf("&&&&&&&&&&&&&&&&&&%s\n", dst);
   //printf("extract weather json: extract weather json success.\n");
   return dst;
}

//解析一天的天气预报,预报信息来自于www.weather.com.cn/data/cityinfo/城市id.html
char *parse_json(char *src)
{
   //printf("parse json: try to parse json data.\n");
   char *city, *cityid, *temp1, *temp2, *weather;
   char *result = (char *)malloc(MAX);
   if (result == NULL)
     printf("malloc to result failed.\n");
   cJSON *root, *format;
   root = cJSON_Parse(src);
   format = cJSON_GetObjectItem(root,"weatherinfo");  
   city = cJSON_GetObjectItem(format,"city")->valuestring;  
   cityid = cJSON_GetObjectItem(format,"cityid")->valuestring;  
   temp1 = cJSON_GetObjectItem(format,"temp1")->valuestring;  
   temp2 = cJSON_GetObjectItem(format,"temp2")->valuestring;  
   weather = cJSON_GetObjectItem(format,"weather")->valuestring;  
   
   strcat(city, ", ");  
   strcat(city, cityid);  
   strcat(city, ", ");  
   strcat(city, "low temprature:");  
   strcat(city, temp1);  
   strcat(city, ", ");  
   strcat(city, "high temprature:");  
   strcat(city, temp2);  
   strcat(city, ", ");  
   strcat(city, weather);  
   strcpy(result, city);  
   //printf("parsejson success.\n");  
   return result;  
}

char *parse_json_local_cityname(char *src)
{
   //printf("parse json: try to parse json data.\n");
   char *city;
   //char *result = (char *)malloc(MAX);

   cJSON *root, *format;
   root = cJSON_Parse(src);
   city = cJSON_GetObjectItem(root,"city")->valuestring;  

   //printf("parsejson success.\n");  
   return city;  
}

//解析六天的天气预报，预报信息来自于http://m.weather.com.cn/data/1城市id.html
char *parse_json_more(char *src)
{
   //printf("parse json: try to parse json data.\n");
   char *city, *cityid, *date_y, *week, *temp1, *temp2,*temp3, *temp4, *temp5, *temp6, *weather1, *weather2, *weather3, *weather4, *weather5, *weather6, *wind1, *wind2, *wind3, *wind4, *wind5, *wind6;
   char *result = (char *)malloc(MAX);
   if (result == NULL)
     printf("malloc to result failed.\n");
   cJSON *root, *format;
   root = cJSON_Parse(src);
   format = cJSON_GetObjectItem(root,"weatherinfo");  
   city = cJSON_GetObjectItem(format,"city")->valuestring;  
   cityid = cJSON_GetObjectItem(format,"cityid")->valuestring;
   date_y = cJSON_GetObjectItem(format,"date_y")->valuestring;   
   week = cJSON_GetObjectItem(format,"week")->valuestring;
   temp1 = cJSON_GetObjectItem(format,"temp1")->valuestring;  
   temp2 = cJSON_GetObjectItem(format,"temp2")->valuestring;
   temp3 = cJSON_GetObjectItem(format,"temp3")->valuestring; 
   temp4 = cJSON_GetObjectItem(format,"temp4")->valuestring; 
   temp5 = cJSON_GetObjectItem(format,"temp5")->valuestring; 
   temp6 = cJSON_GetObjectItem(format,"temp6")->valuestring;   
   weather1 = cJSON_GetObjectItem(format,"weather1")->valuestring;  
   weather2 = cJSON_GetObjectItem(format,"weather2")->valuestring;
   weather3 = cJSON_GetObjectItem(format,"weather3")->valuestring;
   weather4 = cJSON_GetObjectItem(format,"weather4")->valuestring;
   weather5 = cJSON_GetObjectItem(format,"weather5")->valuestring;
   weather6 = cJSON_GetObjectItem(format,"weather6")->valuestring;
   wind1 = cJSON_GetObjectItem(format,"wind1")->valuestring;
   wind2 = cJSON_GetObjectItem(format,"wind2")->valuestring;
   wind3 = cJSON_GetObjectItem(format,"wind3")->valuestring;
   wind4 = cJSON_GetObjectItem(format,"wind4")->valuestring;
   wind5 = cJSON_GetObjectItem(format,"wind5")->valuestring;
   wind6 = cJSON_GetObjectItem(format,"wind6")->valuestring;
   strcat(city, " ");  
   strcat(city, date_y);  
   strcat(city, "\t"); 
   strcat(city, week); 
 
   strcat(city, "\n\t");  
   strcat(city, temp1);  
   strcat(city, " ");  
   strcat(city, weather1);  
   strcat(city, " ");  
   strcat(city, wind1);  

   strcat(city, "\n\t");  
   strcat(city, temp2);  
   strcat(city, " ");  
   strcat(city, weather2);  
   strcat(city, " ");  
   strcat(city, wind2);
  
   strcat(city, "\n\t");  
   strcat(city, temp3);  
   strcat(city, " ");  
   strcat(city, weather3);  
   strcat(city, " ");  
   strcat(city, wind3); 
 
   strcat(city, "\n\t");  
   strcat(city, temp4);  
   strcat(city, " ");  
   strcat(city, weather4);  
   strcat(city, " ");  
   strcat(city, wind4); 
 
   strcat(city, "\n\t");  
   strcat(city, temp5);  
   strcat(city, " ");  
   strcat(city, weather5);  
   strcat(city, " ");  
   strcat(city, wind5); 
 
   strcat(city, "\n\t");  
   strcat(city, temp6);  
   strcat(city, " ");  
   strcat(city, weather6);  
   strcat(city, " ");  
   strcat(city, wind6);
   strcat(result, city);   
   return result;  
}

// get local city ID, example:成都的ID
char *extract_local_city_id()
{
   char *temp = http_request("", "/g/", "61.4.185.48", 81);
   //printf("%s\n", temp);
   char *result = extract_weather_json(temp, 9);
   //printf("%s\n", result);
   char *p1 = "id=";
   char *p2 = strstr(result,p1);
  // printf("%s\n",p2);
   char *p4= "=";
   char *p3 = strstr(p2,p4);
   //printf("%s\n",p3+1);
   
   strncpy(local_city_id, p3+1, 9);
  // printf("%s\n", local_city_id);
   return local_city_id;
}

//write_cache和read_cache用于存和读解析好的天气预报信息，以城市的名称作为文件名，如“chengdu.cache”
void write_cache(char *src, char *name, size_t count)
{
   mkdir("dataWeather", 0777);
   char *file_path = (char *)malloc(30);
   if (file_path == NULL)
     printf("malloc to file_path failed.\n");
   strcpy(file_path, "./dataWeather/");
   strcat(file_path, name);
   int fd = open(file_path, O_RDWR|O_CREAT, S_IRWXU);
   if (fd < 0)
   {
	printf("write cache: open failed.\n");
        exit(1);
   }
   ssize_t ret = write(fd, src, count);
   if (ret < 0)
   {
	printf("write cache: write failed.\n");
	exit(1);
   }
   close(fd);
}

char *read_cache(char *name)
{
   int fd = open(name, O_RDWR);
   if (fd < 0)
   {
	printf("read cache: open failed.\n");
        return NULL;
   }
   ssize_t ret;
   char *dst = (char *)malloc(MAX);
   if (dst == NULL)
     printf("malloc to dst failed.\n");
   ret = read(fd, dst, MAX);
   if (ret < 0)
   {
	printf("read cache: read failed.\n");
	return NULL;
   }
   return dst;
}

/*get local weather, 获取计算机所在地的天气信息，
 *但是存在小bug，获取天气信息速度较慢且并不能保证每次都能成功连接并获取信息；
 *get_local_weather2()该函数可以准确迅速获取当地信息. function one:
*/
void get_local_weather()
{
   char *file_name = "chengdu.cahe";
   //int exist = access(file_name, 0);
   
   printf("local report:\n");
   char *local_city_ID = extract_local_city_id();
   //printf("%s\n", local_city_ID);
   char *ss = strcat(local_city_ID, ".html");
   char *city_weather_info = http_request(ss, "/data/cityinfo/", "61.4.185.201", 80);
   char *city_weather_json = extract_weather_json(city_weather_info, 8);
   char *weather = parse_json(city_weather_json);  
   printf("%s\n", weather);  
   
   ssize_t  weather_len = strlen(weather);
   write_cache(weather, file_name, weather_len);
}

//get local weather, function two:
void get_local_weather2()
{
   char *pp = (char *)malloc(MAX);
   if (pp == NULL)
     printf("malloc to pp failed.\n");
   memset(pp, 0, MAX);
   char *city_id_json = (char *)malloc(MAX);
   if (city_id_json == NULL)
     printf("malloc to city_id_json failed.\n");
   memset(city_id_json, 0, MAX);
   char *city_weather_info = (char *)malloc(MAX);
   if (city_weather_info == NULL)
     printf("malloc to city_weather_info failed.\n");
   memset(city_weather_info, 0, MAX);
   
   pp = http_request("iplookup.php?format=json", "/iplookup/",
"int.dpool.sina.com.cn", 80);
   //printf("11111111111111%s\n",pp);
   char *t = extract_weather_json(pp, 14);
   //printf("%s\n", t);
   char *city_name = parse_json_local_cityname(t);
   //printf("*********%s********\n", city_name); 
   
   city_id_json = http_request(city_name, "/search?cityname=",
"61.4.185.213", 80);
   //printf("22222222222222%s\n",city_id_json);
   char *cityid;
   cityid = extract_city_id(city_id_json);
   char *ss = strcat(cityid, ".html");
   city_weather_info = http_request(ss, "/data/cityinfo/", "61.4.185.201",
80);
   //printf("333333333333333city_weather_info\n%s\n", city_weather_info);

   char *city_weather_json = extract_weather_json(city_weather_info, 8);
   //printf("city_weather_json\n%s\n",city_weather_json);
   char *weather = parse_json(city_weather_json);
   printf("%s\n", weather);
   
   char *city_name1 = city_name;
   //printf("%s\n", city_name1);
   char *file_name = strcat(city_name1, ".cache");
   
   free(pp);
   free(city_id_json);
   free(city_weather_info);
}

/*可以根据用户的要求获取某地的天气信息，如输入“大连”或“dalian”，可以查询大连的天气信息
 *该函数获取一天的信息。
 *get_weather_all_city2()可以获取六天的天气信息。main函数中直接调用这两个函数即可得到天气信息。
*/
void get_weather_all_city(char *city_name)
{
   //char city_name[MAX]; 
   //memset(&city_name, 0, sizeof(city_name));
   //printf("Input city name:");
   //fgets(city_name, MAX, stdin);
   //printf("weather:%s \n", city_name);
   city_name[strlen(city_name)] = '\0';
   char *city_name1 = (char *)malloc(MAX);
   strcpy(city_name1, city_name);
   //printf("city_name1: %s\n", city_name1);
   //char *file_name = (char *)malloc(MAX);
   char *file_name = strcat(city_name1, ".cache");
   //printf(" filename:%s\n", file_name);
   char *city_id_json = http_request(city_name, "/search?cityname=", "61.4.185.213", 80);
   //printf("%s\n",city_id_json);
   char *cityid;
   cityid = extract_city_id(city_id_json);
   //printf("cityid: %s\n", cityid);
   char *ss = strcat(cityid, ".html");
   
   char *city_weather_info = http_request(ss, "/data/cityinfo/", "61.4.185.201", 80);
   //char *city_weather_info = http_request(ss, "/data/", "113.108.239.118 ", 80);  //or IP:113.108.239.107
   //printf("city_weather_info\n%s\n", city_weather_info);

   char *city_weather_json = extract_weather_json(city_weather_info, 9);
   //printf("city_weather_json\n%s\n",city_weather_json);
   char *weather = parse_json(city_weather_json);
   printf("%s\n", weather);
   
   ssize_t  weather_len = strlen(weather);
   write_cache(weather, file_name, weather_len);

}

void get_weather_all_city2()
{
   char city_name[MAX]; 
   printf("Input city name:");
   fgets(city_name, MAX, stdin);
   printf("weather:%s \n", city_name);
   city_name[strlen(city_name) - 1] = '\0';
   char *city_name1 = (char *)malloc(MAX);
   strcpy(city_name1, city_name);
   //printf("city_name1: %s<<<<\n", city_name1);
   //char *file_name = (char *)malloc(MAX);
   char *file_name = strcat(city_name1, ".cache");
   //printf(" filename:%s\n", file_name);
   char *city_id_json = http_request(city_name, "/search?cityname=",
"61.4.185.213", 80);
   
   //printf("%s\n",city_id_json);
   char *cityid;
   cityid = extract_city_id(city_id_json);
   char *ss = strcat(cityid, ".html");
  
   char *city_weather_info = http_request(ss, "/data/", "m.weather.com.cn", 80);
   //char *city_weather_info = http_request(ss, "/data/", "113.108.239.118 ", 80);  //or IP:113.108.239.107
   printf("city_weather_info\n%s\n", city_weather_info);
   char *city_weather_json = extract_weather_json(city_weather_info, 9);
   //printf("city_weather_json\n%s\n",city_weather_json);
   char *weather = parse_json_more(city_weather_json);
   printf("%s\n", weather);

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

void get_news()
{
   char *re = (char *)malloc(MAX);
   memset(re, 0, MAX);
   char *out = (char *)malloc(MAX);
   memset(out, 0, MAX);
   re = http_request("rollnews_ch_out_interface.php","/interface/",
	     "roll.news.sina.com.cn", 80);
   
   g2u(re, strlen(re), out, MAX);
   printf("%s\n", out);
  
   char *new_json = extract_weather_json(out, 19);
  
   //printf("%s\n", new_json);
   free(re);
   free(out);
}

int main(int argc, char *argv[]) 
{
  if (argc != 2) {
    fprintf(stderr, "Error: get cityname.\n");
    exit(1);
  }
  char *city_name;
  city_name = argv[1];
  get_weather_all_city(city_name);
   //test http request;

   //printf("%s\n", http_request("101010100.html", "/data/cityinfo/", "www.weather.com.cn", 80));  
   //printf("%s\n", http_request("", "/2/statuses/public_timeline.json?access_token=2.00VzFRXDS3gX3Cd3a6d28cddjGDizB", "api.weibo.com", 443));      
   
   /*
   char out[MAX];
   int rec = g2u(re, strlen(re), out, 3000);
   printf("%s", *out);
   */
   // extract_city_id  
   /*
   char *temp;  
   char *result;  
   temp = http_request("成都","/search?cityname=","61.4.185.213",80);  
   result = extract_city_id(temp);  
   printf("%s\n", result);  
   */

   // test extract_weather_json  
   //char *temp;  
   //char *result;  
   //temp = http_request("101010100.html", "/data/cityinfo/", "61.4.185.201", 80);  
   //result = extract_weather_json(temp);  
   //printf("%s\n", result);

   //test parse_json  
   //char *temp;  
   //char *result;  
   //char *weather;  
   //temp = http_request("101030100.html", "/data/cityinfo/", "61.4.185.201", 80);  
   //result = extract_weather_json(temp);  
   //weather = parse_json(result);  
   //printf("%s\n", weather);   

   // test write_cache  
   //char *str = "hello, world!";  
   //write_cache(str, "hello.txt", 22);   
 
   /*
   printf("%s\n",http_request("", "/dyndns/getip", "61.160.239.28", 80));
   printf("_______________________________\n");
   char *temp;
   char *result;
   temp = http_request("", "/dyndns/getip", "61.160.239.28", 80);
   result = extract_weather_json(temp);
   printf("%s\n", result);
   */
   
   /*
   char *temp = http_request("", "/g/", "61.4.185.48", 81);
   printf("%s\n", temp);
   char *result = extract_weather_json(temp, 9);
   printf("%s\n", result);
   char *p1 = "id=";
   char *p2 = strstr(result,p1);
   printf("%s\n",p2);
   char *p4= "=";
   char *p3 = strstr(p2, p4);
   printf("%s\n",p3+1);
   char p[14];
   strncpy(p,p3+1,9);
   printf("%s\n",p);
   char *p6 = strcat(p,".html");
   printf("%s\n", http_request(p6, "/data/cityinfo/", "61.4.185.201", 80));  
   */
   //get_news();
   //get_local_weather2();
   //get_weather_all_city();
   
}
