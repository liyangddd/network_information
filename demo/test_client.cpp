#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;
#define MAX 20

int main ()
{
  cout << "-----------------------------------" << endl;
  cout << "Input your choice:" << endl;
  cout << "    1: acquire weibo information" << endl;
  cout << "    2: acquire weather information" << endl;
  cout << "    3: acquire news information" << endl;
  cout << "-----------------------------------" << endl;
  
  int choice;
  while (cout <<"Input your choice: "&&cin >> choice) {
    switch (choice) {
      case 1:
	system("python send_weibo.py");
	break;
      case 2:
	system("./weather");
	break;
      case 3:
	system("python GetNews.py");
	break;
      default:
	cout << "Error input." << endl;
	break;
    }
  }
  return 0;
}
