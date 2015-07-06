#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

void GetWeather();
char* com = "./weather beijing";
int main() {
  
  signal(SIGALRM, GetWeather);
  struct itimerval tick;
  tick.it_value.tv_sec = 2;
  tick.it_value.tv_usec = 0;
  tick.it_interval.tv_sec = 0;
  tick.it_interval.tv_usec = 0;
  
  int ret;
  if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
    perror("Set timer error\n");
    exit(1);
  }
   while (1) {pause();}
}
void GetWeather() {
  //execl("/home/yang/C++/test_client/weather", "beijing", (char *)0);
  execlp("gcc", "gcc", "difftime.c", "-o", "diff", (char *)0);
  
  execlp("/home/yang/C++/test_client/diff", "diff", (char *)0);
  printf("ssss\n");
  //execlp("/home/yang/C++/test_client/GetNews.py", "GetNews.py", (char *)0);
  //system(com);
}
