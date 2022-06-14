#include <iostream>
#include "sha512.h"
 
using std::string;
using std::cout;
using std::endl;
 
int main(int argc, char *argv[])
{
    string input = "blueberries";
    string output1 = sha512(input);
 
    cout << "sha512('"<< input << "')" << output1 << endl;
    return 0;
}

//run command: g++ sha512.cpp main.cpp -o sha512_example && ./sha512_example