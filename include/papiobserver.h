#ifndef PAPIOBSERVER
#define PAPIOBSERVER

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <map>
#include <iostream>

template <int EventCount>
class PapiObserver
{
private:
    int retval;
    int EventSet = PAPI_NULL;
    int eventCounter = 0;
    long long values[EventCount];
    std::map<int, int> eventmap;
    bool running = false;

public:
    PapiObserver(bool init = true)
    {
        if (init)
        {
            /* Initialize the PAPI library */
            retval = PAPI_library_init(PAPI_VER_CURRENT);
            if (retval != PAPI_VER_CURRENT)
            {
                fprintf(stderr, "PAPI library init error!\n");
                exit(1);
            }

            /* Create the Event Set */
            if (PAPI_create_eventset(&EventSet) != PAPI_OK)
                handle_error(1, "Could not create event set");
        }
    }

    void addEvent(int EventCode, char *name)
    {
        if (running)
        {
            fprintf(stderr, "You can't add events while Papi is running\n");
            exit(1);
        }

        if (eventCounter >= EventCount)
        {
            fprintf(stderr, "You have exceeded the custom maximal number of events\n");
            exit(1);
        }

        auto ret = PAPI_add_event(EventSet, EventCode);
        if (ret != PAPI_OK) {
            std::cerr << "failed to add event " << name << ", it returned " << ret << "\n";
            handle_error(1, name);
        }

        eventmap.insert(std::pair<int, int>(EventCode, eventCounter));

        eventCounter++;
    }

    void start()
    {
        /* Start counting */
        if (PAPI_start(EventSet) != PAPI_OK)
            handle_error(1, "Could not start PAPI");

        running = true;
    }

    void stop()
    {
        /*Stop Counting */
        if (PAPI_stop(EventSet, values) != PAPI_OK)
            handle_error(1, "Could not stop PAPI");

        running = false;
    }

    long getResult(int EventCode)
    {
        if (running)
        {
            fprintf(stderr, "You can't get results while Papi is running\n");
            exit(1);
        }

        auto it = eventmap.find(EventCode);
        if (it == eventmap.end())
        {
            fprintf(stderr, "This Event hasn't been added\n");
            exit(1);
        }
        else
        {
            return values[it->second];
        }
    }

    void handle_error(int retval, const char *msg)
    {
        printf("PAPI error %d: There were problems with %s\n", retval, msg);
        exit(1);
    }

    void initCacheAndIdleEvents()
    {
        if (EventSet == PAPI_NULL)
        {
            /* Create the Event Set */
            if (PAPI_create_eventset(&EventSet) != PAPI_OK)
                handle_error(1, "Could not create event set");
        }

        addEvent(PAPI_L3_TCA, "PAPI_L3_TCA");
        addEvent(PAPI_L3_TCM, "PAPI_L3_TCM");
        addEvent(PAPI_RES_STL, "PAPI_RES_STL");
        addEvent(PAPI_MEM_WCY, "PAPI_MEM_WCY");
        addEvent(PAPI_TOT_CYC, "PAPI_TOT_CYC");
    }

    void printCacheAndIdleEvents()
    {
        std::cout << "PAPI=> L3 accesses: " << getResult(PAPI_L3_TCA) << std::endl;
        std::cout << "PAPI=> L3 misses: " << getResult(PAPI_L3_TCM) << std::endl;
        std::cout << "PAPI=> L3 miss/access ratio: " << (double)getResult(PAPI_L3_TCM) / getResult(PAPI_L3_TCA) << std::endl;

        std::cout << "PAPI=> Cycles stalled on any resource: " << getResult(PAPI_RES_STL) << std::endl;
        std::cout << "PAPI=> Cycles stalled on Memory write: " << getResult(PAPI_MEM_WCY) << std::endl;
        std::cout << "PAPI=> Total cycles: " << getResult(PAPI_TOT_CYC) << std::endl;
        std::cout << "PAPI=> stall ratio: " << (double)getResult(PAPI_RES_STL) / getResult(PAPI_TOT_CYC) << std::endl;

        std::cout << "@@#PAPI " << getResult(PAPI_L3_TCA) << " " << getResult(PAPI_L3_TCM) << " " << getResult(PAPI_RES_STL) << " " << getResult(PAPI_MEM_WCY) << " " << getResult(PAPI_TOT_CYC) << std::endl;
    }
};
#endif
