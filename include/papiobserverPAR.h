#ifndef PAPIOBSERVERPAR
#define PAPIOBSERVERPAR

#include <stdlib.h>
#include <stdio.h>
#include "papiobserver.h"
#include <vector>
#include <omp.h>

unsigned long omp_get_thread_num_wrapper(void)
{
    return (unsigned long)omp_get_thread_num();
}

template <int EventCount>
class PapiObserverPar
{
private:
    std::vector<PapiObserver<EventCount>> localPapi;
    int _threadCount;
    std::vector<std::map<int, long long>> values;
    std::vector<bool> running;
    std::vector<bool> finnishedThread;

    void handle_error(int retval, const char *msg)
    {
        printf("PAPI error %d: There were problems with %s\n", retval, msg);
        exit(1);
    }

public:
    PapiObserverPar(int threadCount) : _threadCount(threadCount), values(threadCount), running(threadCount, false), finnishedThread(threadCount, false)
    {
        /* Initialize the PAPI library */
        int retval = PAPI_library_init(PAPI_VER_CURRENT);
        if (retval != PAPI_VER_CURRENT)
        {
            fprintf(stderr, "PAPI library init error!\n");
            exit(1);
        }

        if (threadCount > 1)
        {
            if (PAPI_thread_init(omp_get_thread_num_wrapper) != PAPI_OK)
                handle_error(1, "Could not initialize OMP Support");
            else
                std::cout << "Papi Parallel support enabled" << std::endl;
        }

        for (int i = 0; i < _threadCount; i++)
        {
            values[i].insert(std::pair<int, long long>(PAPI_L3_TCA, 0));
            values[i].insert(std::pair<int, long long>(PAPI_L3_TCM, 0));
            values[i].insert(std::pair<int, long long>(PAPI_RES_STL, 0));
            values[i].insert(std::pair<int, long long>(PAPI_MEM_WCY, 0));
            values[i].insert(std::pair<int, long long>(PAPI_TOT_CYC, 0));
        }
    }

    void createThreadGroup(int n)
    {
        for (int i = 0; i < _threadCount; i++)
            finnishedThread[i] = false;

        localPapi.clear();
        localPapi.reserve(n);
        for (int i = 0; i < n; i++)
        {
            localPapi.push_back(PapiObserver<EventCount>(false));
        }
    }

    void start(int tid = 0)
    {
        if (finnishedThread[tid])
        {
            fprintf(stderr, "Already Finnished! You have to create a new PThreadGroup first!\n");
            exit(1);
        }

        localPapi[tid].initCacheAndIdleEvents();
        localPapi[tid].start();
        running[tid] = true;
    }

    void stop(int tid = 0)
    {
        localPapi[tid].stop();

#pragma omp critical
        {
            for (auto &x : values[tid])
                x.second += localPapi[tid].getResult(x.first);
        }

        PAPI_unregister_thread();
        running[tid] = false;
        finnishedThread[tid] = true;
    }

    long getResult(int EventCode)
    {

        if (std::any_of(running.begin(), running.end(), [](bool val) { return val; }))
        {
            fprintf(stderr, "You can't get results while Papi is running\n");
            exit(1);
        }

        long acc = 0;

        for (auto eventmap : values)
        {
            auto it = eventmap.find(EventCode);
            if (it == eventmap.end())
            {
                fprintf(stderr, "This Event hasn't been added\n");
                exit(1);
            }
            else
            {
                acc += it->second;
            }
        }

        return acc;
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
