#include <iostream>
#include "../include/papiwrapper.h"

int main()
{
    PapiWrapper wpapi;
    wpapi.Init();
    wpapi.AddEvent(PAPI_FSQ_INS_idx);
    std::cout << "Hello World!" << std::endl;
}