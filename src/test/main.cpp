#include <iostream>

extern void lexorTest();
extern void passTest();
extern void nodeTest();

int main(int,const char*[])
{
   lexorTest();
   passTest();
   nodeTest();
   std::cout << "ok" << std::endl;
   return 0;
}
