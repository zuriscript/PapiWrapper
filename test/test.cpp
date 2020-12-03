#include <iostream>
#include "../include/papiwrapper.h"

int main()
{

    PAPIW::INIT(PAPI_FSQ_INS_idx);
    PAPIW::START();
    std::cout << "Hello World!" << std::endl;
    PAPIW::STOP();
}