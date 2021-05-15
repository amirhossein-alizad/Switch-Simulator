#include "Manager.hpp"

#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{   
    Manager* m = new Manager();
    m->handleCommand();
    return 0;
}
