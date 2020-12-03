#include <iostream>
#include "../include/papiwrapper.h"

int main()
{
    PAPIW::INIT(PAPI_L3_TCA);
    PAPIW::START();
    std::cout << "Hello World!" << std::endl;
    PAPIW::STOP();
}