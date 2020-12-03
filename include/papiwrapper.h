#ifndef PAPIWRAPPER
#define PAPIWRAPPER

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <vector>
#include <iostream>
#include <algorithm>

class PapiWrapper
{
private:
    int retval;
    int eventSet = PAPI_NULL;
    std::vector<long long> values;
    std::vector<int> addedEvents;
    std::vector<int> actualEvents;
    bool running = false;

    void handle_error(const char *msg, int retval = PAPI_OK)
    {
        if (retval == PAPI_OK)
            fprintf(stderr, "PAPI ERROR: %s\n", msg);
        else
            fprintf(stderr, "PAPI ERROR (Code %d): %s\n", retval, msg);

        exit(1);
    }

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
        static_assert(is_all_enum<PapiCodes...>(), "All parameters to Init must be of type enum");
        int args[]{eventcodes...}; //unpack
        for (auto eventcode : args)
            AddEvent(eventcode);
    }

    void AddEvent(const int eventCode)
    {
        if (running)
            handle_error("You can't add events while Papi is running\n");

        addedEvents.push_back(eventCode);

        auto ret = PAPI_add_event(eventSet, eventCode);
        if (ret != PAPI_OK)
            std::cerr << "WARNING: Failed to add event " << getDescription(eventCode) << ". It returned " << ret << "\n";
        else
            actualEvents.push_back(eventCode);
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
        retval = PAPI_stop(eventSet, values.data());
        if (retval != PAPI_OK)
            handle_error("Could not stop PAPI", retval);

        running = false;
    }

    long GetResult(int eventCode)
    {
        if (running)
            handle_error("You can't get results while Papi is running\n");

        auto indexInResult = std::find(actualEvents.begin(), actualEvents.end(), eventCode);
        if (indexInResult == actualEvents.end())
            handle_error("The event is not supported or has not been added to the set");

        return values[*indexInResult];
    }

    void Print()
    {
        auto event = actualEvents.begin();
        for (auto addedEvent : addedEvents)
        {
            if (event == actualEvents.end() || *event != addedEvent)
                std::cout << getDescription(addedEvent) << ": NOT SUPPORTED" << std::endl;
            else
            {
                std::cout << getDescription(addedEvent) << ": " << GetResult(addedEvent) << std::endl;
                event++;
            }
        }

        std::cout << "@%@ ";
        for (auto val : values)
            std::cout << val << " ";
        std::cout << std::endl;
    }

private:
    /* For variadic type check*/
    template <typename... Ts>
    struct is_all_enum : std::conjunction<std::is_enum<Ts>...>
    {
    };

    const char *getDescription(int eventCode)
    {
        switch (eventCode)
        {
        case PAPI_L1_DCM_idx:
            return "PAPI_L1_DCM_idx (Level 1 data cache misses)";
        case PAPI_L1_ICM_idx:
            return "PAPI_L1_ICM_idx (Level 1 instruction cache misses)";
        case PAPI_L2_DCM_idx:
            return "PAPI_L2_DCM_idx (Level 2 data cache misses)";
        case PAPI_L2_ICM_idx:
            return "PAPI_L2_ICM_idx (Level 2 instruction cache misses)";
        case PAPI_L3_DCM_idx:
            return "PAPI_L3_DCM_idx (Level 3 data cache misses)";
        case PAPI_L3_ICM_idx:
            return "PAPI_L3_ICM_idx (Level 3 instruction cache misses)";
        case PAPI_L1_TCM_idx:
            return "PAPI_L1_TCM_idx (Level 1 total cache misses)";
        case PAPI_L2_TCM_idx:
            return "PAPI_L2_TCM_idx (Level 2 total cache misses)";
        case PAPI_L3_TCM_idx:
            return "PAPI_L3_TCM_idx (Level 3 total cache misses)";
        case PAPI_CA_SNP_idx:
            return "PAPI_CA_SNP_idx (Snoops)";
        case PAPI_CA_SHR_idx:
            return "PAPI_CA_SHR_idx (Request for shared cache line (SMP))";
        case PAPI_CA_CLN_idx:
            return "PAPI_CA_CLN_idx (Request for clean cache line (SMP))";
        case PAPI_CA_INV_idx:
            return "PAPI_CA_INV_idx (Request for cache line Invalidation (SMP))";
        case PAPI_CA_ITV_idx:
            return "PAPI_CA_ITV_idx (Request for cache line Intervention (SMP))";
        case PAPI_L3_LDM_idx:
            return "PAPI_L3_LDM_idx (Level 3 load misses)";
        case PAPI_L3_STM_idx:
            return "PAPI_L3_STM_idx (Level 3 store misses)";
        case PAPI_BRU_IDL_idx:
            return "PAPI_BRU_IDL_idx (Cycles branch units are idle)";
        case PAPI_FXU_IDL_idx:
            return "PAPI_FXU_IDL_idx (Cycles integer units are idle)";
        case PAPI_FPU_IDL_idx:
            return "PAPI_FPU_IDL_idx (Cycles floating point units are idle)";
        case PAPI_LSU_IDL_idx:
            return "PAPI_LSU_IDL_idx (Cycles load/store units are idle)";
        case PAPI_TLB_DM_idx:
            return "PAPI_TLB_DM_idx (Data translation lookaside buffer misses)";
        case PAPI_TLB_IM_idx:
            return "PAPI_TLB_IM_idx (Instr translation lookaside buffer misses)";
        case PAPI_TLB_TL_idx:
            return "PAPI_TLB_TL_idx (Total translation lookaside buffer misses)";
        case PAPI_L1_LDM_idx:
            return "PAPI_L1_LDM_idx (Level 1 load misses)";
        case PAPI_L1_STM_idx:
            return "PAPI_L1_STM_idx (Level 1 store misses)";
        case PAPI_L2_LDM_idx:
            return "PAPI_L2_LDM_idx (Level 2 load misses)";
        case PAPI_L2_STM_idx:
            return "PAPI_L2_STM_idx (Level 2 store misses)";
        case PAPI_BTAC_M_idx:
            return "PAPI_BTAC_M_idx (BTAC miss)";
        case PAPI_PRF_DM_idx:
            return "PAPI_PRF_DM_idx (Prefetch data instruction caused a miss)";
        case PAPI_L3_DCH_idx:
            return "PAPI_L3_DCH_idx (Level 3 Data Cache Hit)";
        case PAPI_TLB_SD_idx:
            return "PAPI_TLB_SD_idx (Xlation lookaside buffer shootdowns (SMP))";
        case PAPI_CSR_FAL_idx:
            return "PAPI_CSR_FAL_idx (Failed store conditional instructions)";
        case PAPI_CSR_SUC_idx:
            return "PAPI_CSR_SUC_idx (Successful store conditional instructions)";
        case PAPI_CSR_TOT_idx:
            return "PAPI_CSR_TOT_idx (Total store conditional instructions)";
        case PAPI_MEM_SCY_idx:
            return "PAPI_MEM_SCY_idx (Cycles Stalled Waiting for Memory Access)";
        case PAPI_MEM_RCY_idx:
            return "PAPI_MEM_RCY_idx (Cycles Stalled Waiting for Memory Read)";
        case PAPI_MEM_WCY_idx:
            return "PAPI_MEM_WCY_idx (Cycles Stalled Waiting for Memory Write)";
        case PAPI_STL_ICY_idx:
            return "PAPI_STL_ICY_idx (Cycles with No Instruction Issue)";
        case PAPI_FUL_ICY_idx:
            return "PAPI_FUL_ICY_idx (Cycles with Maximum Instruction Issue)";
        case PAPI_STL_CCY_idx:
            return "PAPI_STL_CCY_idx (Cycles with No Instruction Completion)";
        case PAPI_FUL_CCY_idx:
            return "PAPI_FUL_CCY_idx (Cycles with Maximum Instruction Completion)";
        case PAPI_HW_INT_idx:
            return "PAPI_HW_INT_idx (Hardware interrupts)";
        case PAPI_BR_UCN_idx:
            return "PAPI_BR_UCN_idx (Unconditional branch instructions executed)";
        case PAPI_BR_CN_idx:
            return "PAPI_BR_CN_idx (Conditional branch instructions executed)";
        case PAPI_BR_TKN_idx:
            return "PAPI_BR_TKN_idx (Conditional branch instructions taken)";
        case PAPI_BR_NTK_idx:
            return "PAPI_BR_NTK_idx (Conditional branch instructions not taken)";
        case PAPI_BR_MSP_idx:
            return "PAPI_BR_MSP_idx (Conditional branch instructions mispred)";
        case PAPI_BR_PRC_idx:
            return "PAPI_BR_PRC_idx (Conditional branch instructions corr. pred)";
        case PAPI_FMA_INS_idx:
            return "PAPI_FMA_INS_idx (FMA instructions completed)";
        case PAPI_TOT_IIS_idx:
            return "PAPI_TOT_IIS_idx (Total instructions issued)";
        case PAPI_TOT_INS_idx:
            return "PAPI_TOT_INS_idx (Total instructions executed)";
        case PAPI_INT_INS_idx:
            return "PAPI_INT_INS_idx (Integer instructions executed)";
        case PAPI_FP_INS_idx:
            return "PAPI_FP_INS_idx (Floating point instructions executed)";
        case PAPI_LD_INS_idx:
            return "PAPI_LD_INS_idx (Load instructions executed)";
        case PAPI_SR_INS_idx:
            return "PAPI_SR_INS_idx (Store instructions executed)";
        case PAPI_BR_INS_idx:
            return "PAPI_BR_INS_idx (Total branch instructions executed)";
        case PAPI_VEC_INS_idx:
            return "PAPI_VEC_INS_idx (Vector/SIMD instructions executed (could include integer))";
        case PAPI_RES_STL_idx:
            return "PAPI_RES_STL_idx (Cycles processor is stalled on resource)";
        case PAPI_FP_STAL_idx:
            return "PAPI_FP_STAL_idx (Cycles any FP units are stalled)";
        case PAPI_TOT_CYC_idx:
            return "PAPI_TOT_CYC_idx (Total cycles executed)";
        case PAPI_LST_INS_idx:
            return "PAPI_LST_INS_idx (Total load/store inst. executed)";
        case PAPI_SYC_INS_idx:
            return "PAPI_SYC_INS_idx (Sync. inst. executed)";
        case PAPI_L1_DCH_idx:
            return "PAPI_L1_DCH_idx (L1 D Cache Hit)";
        case PAPI_L2_DCH_idx:
            return "PAPI_L2_DCH_idx (L2 D Cache Hit)";
        case PAPI_L1_DCA_idx:
            return "PAPI_L1_DCA_idx (L1 D Cache Access)";
        case PAPI_L2_DCA_idx:
            return "PAPI_L2_DCA_idx (L2 D Cache Access)";
        case PAPI_L3_DCA_idx:
            return "PAPI_L3_DCA_idx (L3 D Cache Access)";
        case PAPI_L1_DCR_idx:
            return "PAPI_L1_DCR_idx (L1 D Cache Read)";
        case PAPI_L2_DCR_idx:
            return "PAPI_L2_DCR_idx (L2 D Cache Read)";
        case PAPI_L3_DCR_idx:
            return "PAPI_L3_DCR_idx (L3 D Cache Read)";
        case PAPI_L1_DCW_idx:
            return "PAPI_L1_DCW_idx (L1 D Cache Write)";
        case PAPI_L2_DCW_idx:
            return "PAPI_L2_DCW_idx (L2 D Cache Write)";
        case PAPI_L3_DCW_idx:
            return "PAPI_L3_DCW_idx (L3 D Cache Write)";
        case PAPI_L1_ICH_idx:
            return "PAPI_L1_ICH_idx (L1 instruction cache hits)";
        case PAPI_L2_ICH_idx:
            return "PAPI_L2_ICH_idx (L2 instruction cache hits)";
        case PAPI_L3_ICH_idx:
            return "PAPI_L3_ICH_idx (L3 instruction cache hits)";
        case PAPI_L1_ICA_idx:
            return "PAPI_L1_ICA_idx (L1 instruction cache accesses)";
        case PAPI_L2_ICA_idx:
            return "PAPI_L2_ICA_idx (L2 instruction cache accesses)";
        case PAPI_L3_ICA_idx:
            return "PAPI_L3_ICA_idx (L3 instruction cache accesses)";
        case PAPI_L1_ICR_idx:
            return "PAPI_L1_ICR_idx (L1 instruction cache reads)";
        case PAPI_L2_ICR_idx:
            return "PAPI_L2_ICR_idx (L2 instruction cache reads)";
        case PAPI_L3_ICR_idx:
            return "PAPI_L3_ICR_idx (L3 instruction cache reads)";
        case PAPI_L1_ICW_idx:
            return "PAPI_L1_ICW_idx (L1 instruction cache writes)";
        case PAPI_L2_ICW_idx:
            return "PAPI_L2_ICW_idx (L2 instruction cache writes)";
        case PAPI_L3_ICW_idx:
            return "PAPI_L3_ICW_idx (L3 instruction cache writes)";
        case PAPI_L1_TCH_idx:
            return "PAPI_L1_TCH_idx (L1 total cache hits)";
        case PAPI_L2_TCH_idx:
            return "PAPI_L2_TCH_idx (L2 total cache hits)";
        case PAPI_L3_TCH_idx:
            return "PAPI_L3_TCH_idx (L3 total cache hits)";
        case PAPI_L1_TCA_idx:
            return "PAPI_L1_TCA_idx (L1 total cache accesses)";
        case PAPI_L2_TCA_idx:
            return "PAPI_L2_TCA_idx (L2 total cache accesses)";
        case PAPI_L3_TCA_idx:
            return "PAPI_L3_TCA_idx (L3 total cache accesses)";
        case PAPI_L1_TCR_idx:
            return "PAPI_L1_TCR_idx (L1 total cache reads)";
        case PAPI_L2_TCR_idx:
            return "PAPI_L2_TCR_idx (L2 total cache reads)";
        case PAPI_L3_TCR_idx:
            return "PAPI_L3_TCR_idx (L3 total cache reads)";
        case PAPI_L1_TCW_idx:
            return "PAPI_L1_TCW_idx (L1 total cache writes)";
        case PAPI_L2_TCW_idx:
            return "PAPI_L2_TCW_idx (L2 total cache writes)";
        case PAPI_L3_TCW_idx:
            return "PAPI_L3_TCW_idx (L3 total cache writes)";
        case PAPI_FML_INS_idx:
            return "PAPI_FML_INS_idx (FM ins)";
        case PAPI_FAD_INS_idx:
            return "PAPI_FAD_INS_idx (FA ins)";
        case PAPI_FDV_INS_idx:
            return "PAPI_FDV_INS_idx (FD ins)";
        case PAPI_FSQ_INS_idx:
            return "PAPI_FSQ_INS_idx (FSq ins)";
        case PAPI_FNV_INS_idx:
            return "PAPI_FNV_INS_idx (Finv ins)";
        case PAPI_FP_OPS_idx:
            return "PAPI_FP_OPS_idx (Floating point operations executed)";
        case PAPI_SP_OPS_idx:
            return "PAPI_SP_OPS_idx (Floating point operations executed: optimized to count scaled single precision vector operations)";
        case PAPI_DP_OPS_idx:
            return "PAPI_DP_OPS_idx (Floating point operations executed: optimized to count scaled double precision vector operations)";
        case PAPI_VEC_SP_idx:
            return "PAPI_VEC_SP_idx (Single precision vector/SIMD instructions)";
        case PAPI_VEC_DP_idx:
            return "PAPI_VEC_DP_idx (Double precision vector/SIMD instructions)";
        case PAPI_REF_CYC_idx:
            return "PAPI_REF_CYC_idx (Reference clock cycles)";
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
