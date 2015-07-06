#include"head.h"

#define FILE "weather.txt"

//打印出weather.txt中需要的信息
void print_weather(int _fd)
{
	int fd = _fd;
	//50000可能有点大，但是由于本人很懒，干脆给他个大点，免得到时空间不够
	char buffer[50000];
	int num = 0;
	memset(buffer,0,50000);

	//将文件指针指向到文件开始
	int lp = lseek(fd,4700,SEEK_SET);
	if( (num=read(fd,buffer,50000)) > 0)
		filter(buffer);
	else printf("printmesg read file fail\n");
	return;
		
}

//信息过滤，筛选出今天天气的信息
//这是我自己写的一个过滤函数，写的十分的仓促，因此不是很好，大家可以自己优化一下
void filter(char* buffer)
{
	//按照need的内容筛选
	char* need = {"武汉明天天气预报"};
	//char* need = {"武汉天气指数"};
        int ipoint = 0;

	//while中为还没有读到文件结尾
	while(buffer[ipoint] != '\0')
	{
		//cp即CharPoint的简写
		char* cp;
		if( (cp = strstr(buffer,need)) != NULL)
		{
		
			//context用于存放找到的有用信息
			char context[400] = {0};
			memcpy(context,cp,399);
			int i = 0;
			int j = 0;
			char aresult[400]={0};
			for(i = 0;i<380;i++)
			{
				//在weather.txt中字符编码为UTF-8，而0x20因该代表的是空格
				//此处过滤掉没有用的空格
				if(context[i] != 0x20)
				{
				
					aresult[j] = context[i];
					j++;
				}
			}
			
			i = 0;
			memset(context,0,strlen(context));
			j = 0;
			for(i=0;i<strlen(aresult);i++)
			{
				//同样，此处过滤掉没有用的回车换行
				if(aresult[i] != 0xd && aresult[i] != 0xa)
				{
					context[j] = aresult[i];
					j++;
				}
			}
			//最后将有用的信息打印出来
			printf("%s\n",context);
			break;
		}
		ipoint++;
	}
}
