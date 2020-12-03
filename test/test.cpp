#include <iostream>
#include "../include/papiwrapper.h"
#include <omp.h>
#include <pthread.h>
#include <map>

int main()
{
    PAPIW::INIT(PAPI_L3_TCA);
    PAPIW::START();
    std::cout << "Hello World!" << std::endl;
    PAPIW::STOP();
    PAPIW::PRINT();
}
