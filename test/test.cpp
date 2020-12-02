#include <iostream>
#include "../include/papiwrapper.h"

int main()
{
    PapiWrapper wpapi;
    wpapi.Init(PAPI_FSQ_INS_idx);
    wpapi.Start();
    std::cout << "Hello World!" << std::endl;
    wpapi.Stop();
}