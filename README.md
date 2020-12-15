# PAPIW (A Papi wrapper)

This repository contains the code for a Papi wrapper which simplifies the use of Papi, especially when using Openmp.  
It comes as a Header-only library, which only requires `papiwrapper.h` to be included.  
This will inject the namespace `PAPIW` which provides a slim and functional API without the need to take care of any objects.

### Setup Example

```bash
$ git clone https://github.com/ZurShmaria/PapiWrapper.git && cd PapiWrapper
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j4
```

To run the provided example:

```bash
$ bin/papiw_example
```

### Include

To add the Papi Library, copy `cmake/FindPAPI.cmake` and the folder `include/` to your project.  
Recommended Cmake setup:

```cmake
# Find PAPI on system
find_package(PAPI)

# If present, include directories. Otherwise set in-house NOPAPIW variable
if(PAPI_FOUND)
    include_directories(${PAPI_INCLUDE_DIRS})
else()
    add_compile_definitions(NOPAPIW)
endif(PAPI_FOUND)

# Include PapiWrapper Lib (change the path, if you stored the files it in a different folder)
target_include_directories(${TargetName} INTERFACE include/)

# Link to PAPI if found on system
if(PAPI_FOUND)
        target_link_libraries(${TargetName} ${PAPI_LIBRARIES})
endif(PAPI_FOUND)
```

### Usage

Initialization (Allows variadic Papi eventcode arguments):

```c++
    // Use either
    PAPIW::INIT_SINGLE(PAPI_L2_TCA, PAPI_L1_TCM, PAPI_L3_TCA, PAPI_L3_TCM); // Init PAPIW for sequential use
    // Or
    PAPIW::INIT_PARALLEL(PAPI_L2_TCA, PAPI_L1_TCM, PAPI_L3_TCA, PAPI_L3_TCM); // Init PAPIW for sequential use
```

Benchmarking:

```c++
    PAPIW::START();
    doSomethingInteressting();
    PAPIW::STOP();

    doSomethingUnimportant(); // Do not measure

    PAPIW::START();
    doSomethingInteresstingAgain();
    PAPIW::STOP();
```

Resetting:

```c++
    PAPIW::RESET(); // Set the intermediate counter values to zero
```

Printing:

```c++
    PAPIW::PRINT();
```

The output could look something like:

```
PAPIW Parallel PapiWrapper instance report:
PAPI_L2_TCA (L2 total cache accesses): 68743998
PAPI_TOT_CYC (Total cycles executed): 9800773029
PAPI_L3_TCA (L3 total cache accesses): 32864360
PAPI_L3_TCM (Level 3 total cache misses): 17234237
@%% PAPI_L2_TCA PAPI_TOT_CYC PAPI_L3_TCA PAPI_L3_TCM
@%@ 68743998 9800773029 32864360 17234237
```

See `example.cpp` for more details

### Info

- The recommended cmake setup aims for a soft dependency: If Papi is not available in the system, then most code will not get compiled and any call to `PAPIW` is turned into a No-op. The same effect can be achieved by setting `NOPAPIW` for building
- Since `PAPIW` needs threadprivate states and Papi itself should be informed whenever an underlying kernel LWP was killed, one should stop and start between different parallel regions, whenever possible.
- If `omp_set_num_threads` is used, `PAPIW::STOP()` has to be called right before. Certainly, `PAPIW:START()` may be called immediately afterwards.
- Whenever possible, `PAPIW:START()` and `PAPIW:STOP()` should be called directly inside a parallel region
- If an event, which is not available on the system, is added in `PAPIW:INIT`, then only a warning is displayed and the program continues. Of course no data can be gathered and hence, no data for that specific event can be gathered
- A lot of state checks are used for `PAPIW`. In the event of an invalid state, the program aborts and a human-readable error message is printed out
- The output is optimized for easy extraction, e.g. for some plotting programs:
  - First all observed counters are displayed with a short description and their measured values
  - Then `@%%` indicates the header (Papi Counter name)
  - And `@%@` indicates the values

### Limitations

- Was created with OpenMp in mind
- `omp_set_dynamic` should be false. Otherwise make sure that `PAPIW:START()` and `PAPIW:STOP()` are only used inside Parallel regions
- Do not start `PAPIW` in a parallel region and stop the same run in a sequential region, and vice versa
