#ifndef PAPIWRAPPER
#define PAPIWRAPPER

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <vector>
#include <iostream>
#include <algorithm>

#define PAPIW_MAX 20

class PapiWrapper
{
private:
    int retval;
    int eventSet = PAPI_NULL;
    int maxEventCount = PAPIW_MAX;
    long long buffer[PAPIW_MAX];
    std::vector<int> events;
    bool running = false;

public:
    template <typename... PapiCodes>
    void Init(PapiCodes const... eventcodes)
    {
        /* Initialize the PAPI library */
        retval = PAPI_library_init(PAPI_VER_CURRENT);
        if (retval != PAPI_VER_CURRENT)
            handle_error("PAPI library init error!\n", retval);

        /* Create the Event Set */
        retval = PAPI_create_eventset(&eventSet);
        if (retval != PAPI_OK)
            handle_error("Could not create event set", retval);

        /* Prepare Events */
        static_assert(std::conjunction<std::is_integral<PapiCodes>...>(), "All parameters to Init must be of integral type");
        int args[]{eventcodes...}; //unpack
        for (auto eventcode : args)
            AddEvent(eventcode);
    }

    void AddEvent(const int eventCode)
    {
        if (running)
            handle_error("You can't add events while Papi is running\n");

        if (events.size() >= PAPIW_MAX)
            handle_error("Event count limit exceeded. Check PAPIW_MAX\n");

        auto ret = PAPI_add_event(eventSet, eventCode);
        if (ret != PAPI_OK)
            std::cerr << "WARNING: Failed to add event " << getDescription(eventCode) << ". It returned Code " << ret << "\n";
        else
            events.push_back(eventCode);
    }

    void Start()
    {
        /* Start counting */
        retval = PAPI_start(eventSet);
        if (retval != PAPI_OK)
            handle_error("Could not start PAPI", retval);

        running = true;
    }

    void Stop()
    {
        /*Stop Counting */
        retval = PAPI_stop(eventSet, buffer);
        if (retval != PAPI_OK)
            handle_error("Could not stop PAPI", retval);

        running = false;
    }

    long GetResult(int eventCode)
    {
        if (running)
            handle_error("You can't get results while Papi is running\n");

        auto indexInResult = std::find(events.begin(), events.end(), eventCode);
        if (indexInResult == events.end())
            handle_error("The event is not supported or has not been added to the set");

        return buffer[*indexInResult];
    }

    void Print()
    {
        for (auto eventCode : events)
            std::cout << getDescription(eventCode) << ": " << GetResult(eventCode) << std::endl;

        /* Print Headers */
        std::cout << "@%% ";
        for (auto eventCode : events)
        {
            auto description = getDescription(eventCode);
            for (int j = 0; description[j] != '\0' && description[j] != ' ' && j < 20; j++)
                std::cout << description[j] << " ";
        }
        std::cout << std::endl;

        /* Print results */
        int count = events.size();
        std::cout << "@%@ ";
        for (int i = 0; i < count; i++)
            std::cout << buffer[i] << " ";
        std::cout << std::endl;
    }

private:
    void handle_error(const char *msg, int retval = PAPI_OK)
    {
        if (retval == PAPI_OK)
            fprintf(stderr, "PAPI ERROR: %s\n", msg);
        else
            fprintf(stderr, "PAPI ERROR (Code %d): %s\n", retval, msg);

        exit(1);
    }

    const char *getDescription(int eventCode)
    {
        switch (eventCode)
        {
        case PAPI_L1_DCM:
            return "PAPI_L1_DCM (Level 1 data cache misses)";
        case PAPI_L1_ICM:
            return "PAPI_L1_ICM (Level 1 instruction cache misses)";
        case PAPI_L2_DCM:
            return "PAPI_L2_DCM (Level 2 data cache misses)";
        case PAPI_L2_ICM:
            return "PAPI_L2_ICM (Level 2 instruction cache misses)";
        case PAPI_L3_DCM:
            return "PAPI_L3_DCM (Level 3 data cache misses)";
        case PAPI_L3_ICM:
            return "PAPI_L3_ICM (Level 3 instruction cache misses)";
        case PAPI_L1_TCM:
            return "PAPI_L1_TCM (Level 1 total cache misses)";
        case PAPI_L2_TCM:
            return "PAPI_L2_TCM (Level 2 total cache misses)";
        case PAPI_L3_TCM:
            return "PAPI_L3_TCM (Level 3 total cache misses)";
        case PAPI_CA_SNP:
            return "PAPI_CA_SNP (Snoops)";
        case PAPI_CA_SHR:
            return "PAPI_CA_SHR (Request for shared cache line (SMP))";
        case PAPI_CA_CLN:
            return "PAPI_CA_CLN (Request for clean cache line (SMP))";
        case PAPI_CA_INV:
            return "PAPI_CA_INV (Request for cache line Invalidation (SMP))";
        case PAPI_CA_ITV:
            return "PAPI_CA_ITV (Request for cache line Intervention (SMP))";
        case PAPI_L3_LDM:
            return "PAPI_L3_LDM (Level 3 load misses)";
        case PAPI_L3_STM:
            return "PAPI_L3_STM (Level 3 store misses)";
        case PAPI_BRU_IDL:
            return "PAPI_BRU_IDL (Cycles branch units are idle)";
        case PAPI_FXU_IDL:
            return "PAPI_FXU_IDL (Cycles integer units are idle)";
        case PAPI_FPU_IDL:
            return "PAPI_FPU_IDL (Cycles floating point units are idle)";
        case PAPI_LSU_IDL:
            return "PAPI_LSU_IDL (Cycles load/store units are idle)";
        case PAPI_TLB_DM:
            return "PAPI_TLB_DM (Data translation lookaside buffer misses)";
        case PAPI_TLB_IM:
            return "PAPI_TLB_IM (Instr translation lookaside buffer misses)";
        case PAPI_TLB_TL:
            return "PAPI_TLB_TL (Total translation lookaside buffer misses)";
        case PAPI_L1_LDM:
            return "PAPI_L1_LDM (Level 1 load misses)";
        case PAPI_L1_STM:
            return "PAPI_L1_STM (Level 1 store misses)";
        case PAPI_L2_LDM:
            return "PAPI_L2_LDM (Level 2 load misses)";
        case PAPI_L2_STM:
            return "PAPI_L2_STM (Level 2 store misses)";
        case PAPI_BTAC_M:
            return "PAPI_BTAC_M (BTAC miss)";
        case PAPI_PRF_DM:
            return "PAPI_PRF_DM (Prefetch data instruction caused a miss)";
        case PAPI_L3_DCH:
            return "PAPI_L3_DCH (Level 3 Data Cache Hit)";
        case PAPI_TLB_SD:
            return "PAPI_TLB_SD (Xlation lookaside buffer shootdowns (SMP))";
        case PAPI_CSR_FAL:
            return "PAPI_CSR_FAL (Failed store conditional instructions)";
        case PAPI_CSR_SUC:
            return "PAPI_CSR_SUC (Successful store conditional instructions)";
        case PAPI_CSR_TOT:
            return "PAPI_CSR_TOT (Total store conditional instructions)";
        case PAPI_MEM_SCY:
            return "PAPI_MEM_SCY (Cycles Stalled Waiting for Memory Access)";
        case PAPI_MEM_RCY:
            return "PAPI_MEM_RCY (Cycles Stalled Waiting for Memory Read)";
        case PAPI_MEM_WCY:
            return "PAPI_MEM_WCY (Cycles Stalled Waiting for Memory Write)";
        case PAPI_STL_ICY:
            return "PAPI_STL_ICY (Cycles with No Instruction Issue)";
        case PAPI_FUL_ICY:
            return "PAPI_FUL_ICY (Cycles with Maximum Instruction Issue)";
        case PAPI_STL_CCY:
            return "PAPI_STL_CCY (Cycles with No Instruction Completion)";
        case PAPI_FUL_CCY:
            return "PAPI_FUL_CCY (Cycles with Maximum Instruction Completion)";
        case PAPI_HW_INT:
            return "PAPI_HW_INT (Hardware interrupts)";
        case PAPI_BR_UCN:
            return "PAPI_BR_UCN (Unconditional branch instructions executed)";
        case PAPI_BR_CN:
            return "PAPI_BR_CN (Conditional branch instructions executed)";
        case PAPI_BR_TKN:
            return "PAPI_BR_TKN (Conditional branch instructions taken)";
        case PAPI_BR_NTK:
            return "PAPI_BR_NTK (Conditional branch instructions not taken)";
        case PAPI_BR_MSP:
            return "PAPI_BR_MSP (Conditional branch instructions mispred)";
        case PAPI_BR_PRC:
            return "PAPI_BR_PRC (Conditional branch instructions corr. pred)";
        case PAPI_FMA_INS:
            return "PAPI_FMA_INS (FMA instructions completed)";
        case PAPI_TOT_IIS:
            return "PAPI_TOT_IIS (Total instructions issued)";
        case PAPI_TOT_INS:
            return "PAPI_TOT_INS (Total instructions executed)";
        case PAPI_INT_INS:
            return "PAPI_INT_INS (Integer instructions executed)";
        case PAPI_FP_INS:
            return "PAPI_FP_INS (Floating point instructions executed)";
        case PAPI_LD_INS:
            return "PAPI_LD_INS (Load instructions executed)";
        case PAPI_SR_INS:
            return "PAPI_SR_INS (Store instructions executed)";
        case PAPI_BR_INS:
            return "PAPI_BR_INS (Total branch instructions executed)";
        case PAPI_VEC_INS:
            return "PAPI_VEC_INS (Vector/SIMD instructions executed (could include integer))";
        case PAPI_RES_STL:
            return "PAPI_RES_STL (Cycles processor is stalled on resource)";
        case PAPI_FP_STAL:
            return "PAPI_FP_STAL (Cycles any FP units are stalled)";
        case PAPI_TOT_CYC:
            return "PAPI_TOT_CYC (Total cycles executed)";
        case PAPI_LST_INS:
            return "PAPI_LST_INS (Total load/store inst. executed)";
        case PAPI_SYC_INS:
            return "PAPI_SYC_INS (Sync. inst. executed)";
        case PAPI_L1_DCH:
            return "PAPI_L1_DCH (L1 D Cache Hit)";
        case PAPI_L2_DCH:
            return "PAPI_L2_DCH (L2 D Cache Hit)";
        case PAPI_L1_DCA:
            return "PAPI_L1_DCA (L1 D Cache Access)";
        case PAPI_L2_DCA:
            return "PAPI_L2_DCA (L2 D Cache Access)";
        case PAPI_L3_DCA:
            return "PAPI_L3_DCA (L3 D Cache Access)";
        case PAPI_L1_DCR:
            return "PAPI_L1_DCR (L1 D Cache Read)";
        case PAPI_L2_DCR:
            return "PAPI_L2_DCR (L2 D Cache Read)";
        case PAPI_L3_DCR:
            return "PAPI_L3_DCR (L3 D Cache Read)";
        case PAPI_L1_DCW:
            return "PAPI_L1_DCW (L1 D Cache Write)";
        case PAPI_L2_DCW:
            return "PAPI_L2_DCW (L2 D Cache Write)";
        case PAPI_L3_DCW:
            return "PAPI_L3_DCW (L3 D Cache Write)";
        case PAPI_L1_ICH:
            return "PAPI_L1_ICH (L1 instruction cache hits)";
        case PAPI_L2_ICH:
            return "PAPI_L2_ICH (L2 instruction cache hits)";
        case PAPI_L3_ICH:
            return "PAPI_L3_ICH (L3 instruction cache hits)";
        case PAPI_L1_ICA:
            return "PAPI_L1_ICA (L1 instruction cache accesses)";
        case PAPI_L2_ICA:
            return "PAPI_L2_ICA (L2 instruction cache accesses)";
        case PAPI_L3_ICA:
            return "PAPI_L3_ICA (L3 instruction cache accesses)";
        case PAPI_L1_ICR:
            return "PAPI_L1_ICR (L1 instruction cache reads)";
        case PAPI_L2_ICR:
            return "PAPI_L2_ICR (L2 instruction cache reads)";
        case PAPI_L3_ICR:
            return "PAPI_L3_ICR (L3 instruction cache reads)";
        case PAPI_L1_ICW:
            return "PAPI_L1_ICW (L1 instruction cache writes)";
        case PAPI_L2_ICW:
            return "PAPI_L2_ICW (L2 instruction cache writes)";
        case PAPI_L3_ICW:
            return "PAPI_L3_ICW (L3 instruction cache writes)";
        case PAPI_L1_TCH:
            return "PAPI_L1_TCH (L1 total cache hits)";
        case PAPI_L2_TCH:
            return "PAPI_L2_TCH (L2 total cache hits)";
        case PAPI_L3_TCH:
            return "PAPI_L3_TCH (L3 total cache hits)";
        case PAPI_L1_TCA:
            return "PAPI_L1_TCA (L1 total cache accesses)";
        case PAPI_L2_TCA:
            return "PAPI_L2_TCA (L2 total cache accesses)";
        case PAPI_L3_TCA:
            return "PAPI_L3_TCA (L3 total cache accesses)";
        case PAPI_L1_TCR:
            return "PAPI_L1_TCR (L1 total cache reads)";
        case PAPI_L2_TCR:
            return "PAPI_L2_TCR (L2 total cache reads)";
        case PAPI_L3_TCR:
            return "PAPI_L3_TCR (L3 total cache reads)";
        case PAPI_L1_TCW:
            return "PAPI_L1_TCW (L1 total cache writes)";
        case PAPI_L2_TCW:
            return "PAPI_L2_TCW (L2 total cache writes)";
        case PAPI_L3_TCW:
            return "PAPI_L3_TCW (L3 total cache writes)";
        case PAPI_FML_INS:
            return "PAPI_FML_INS (FM ins)";
        case PAPI_FAD_INS:
            return "PAPI_FAD_INS (FA ins)";
        case PAPI_FDV_INS:
            return "PAPI_FDV_INS (FD ins)";
        case PAPI_FSQ_INS:
            return "PAPI_FSQ_INS (FSq ins)";
        case PAPI_FNV_INS:
            return "PAPI_FNV_INS (Finv ins)";
        case PAPI_FP_OPS:
            return "PAPI_FP_OPS (Floating point operations executed)";
        case PAPI_SP_OPS:
            return "PAPI_SP_OPS (Floating point operations executed: optimized to count scaled single precision vector operations)";
        case PAPI_DP_OPS:
            return "PAPI_DP_OPS (Floating point operations executed: optimized to count scaled double precision vector operations)";
        case PAPI_VEC_SP:
            return "PAPI_VEC_SP (Single precision vector/SIMD instructions)";
        case PAPI_VEC_DP:
            return "PAPI_VEC_DP (Double precision vector/SIMD instructions)";
        case PAPI_REF_CYC:
            return "PAPI_REF_CYC (Reference clock cycles)";
        default:
            return "UNKNOWN CODE";
        }
    }
};

namespace PAPIW
{
    PapiWrapper papiwrapper;

    template <typename... PapiCodes>
    void INIT(PapiCodes const... eventcodes)
    {
        papiwrapper.Init(eventcodes...);
    }

    void START()
    {
        papiwrapper.Start();
    }

    void STOP()
    {
        papiwrapper.Stop();
    }

    void PRINT()
    {
        papiwrapper.Print();
    }
} // namespace PAPIW

#endif
