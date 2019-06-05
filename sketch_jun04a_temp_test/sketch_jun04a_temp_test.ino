#include <stdlib.h>
#include <functional.h>
#include <string.h>

int add(int x, int y) {return x+y;}
int sub(int x, int y) {return x-y;}

int main()
{
    std::function<std::string, std::function<int(int,int)>>  funcMap =
         {{ "add", add},
          { "sub", sub}
         };

//    std::cout << funcMap["add"](2,3) << "\n";
//    std::cout << funcMap["sub"](5,2) << "\n";
}
