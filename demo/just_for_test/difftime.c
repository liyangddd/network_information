#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void main()  
{  
    time_t timep1; //now time
    time_t timep2; 
    struct tm *p_tm1;
    struct tm *p_tm2;
    //memset(&p_tm2, 0, sizeof(p_tm2));
    p_tm2 = (struct tm *)malloc(sizeof(struct tm));
    timep1 = time((time_t *)NULL);
    printf("%s\n", ctime(&timep1));
    printf("time: %d\n", timep1);
    p_tm1 = gmtime(&timep1);
    printf("now time %d %d %d %d %d %d %d %d %d: \n", p_tm1->tm_sec,
p_tm1->tm_min, p_tm1->tm_hour, p_tm1->tm_mday, p_tm1->tm_mon, p_tm1->tm_year,
p_tm1->tm_wday, p_tm1->tm_yday, p_tm1->tm_isdst); 
   
    p_tm2->tm_sec = 22;
    p_tm2->tm_min = 11;
    p_tm2->tm_hour = 01;
    p_tm2->tm_mday = 11;
    p_tm2->tm_mon = 03;
    p_tm2->tm_year = 114;
//     p_tm2->tm_wday = 4;
//     p_tm2->tm_yday = 5;
//     p_tm2->tm_isdst = 0;
    
    printf("111\n");
    timep2 = mktime(p_tm2);
    printf("make: %f\n", difftime(timep1, timep2));
     printf("222\n");
     printf("%s\n", ctime(&timep2));
}
