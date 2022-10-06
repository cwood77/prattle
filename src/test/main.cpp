#include <iostream>

extern void lexorTest();
extern void passTest();

int main(int,const char*[])
{
   lexorTest();
   passTest();
   std::cout << "ok" << std::endl;
   return 0;
}
