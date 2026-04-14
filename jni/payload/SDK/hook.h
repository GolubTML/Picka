#pragma once

#include <stdio.h>
#include <vector>

int is_adrp(uint32_t instr);
uintptr_t calc_adrp_target(uintptr_t pc, uint32_t instr);
int get_adrp_reg(uint32_t instr);
int write_abs_load(uint32_t* dst, int reg, uintptr_t addr);


#ifdef __cplusplus
extern "C" 
{
    #endif
    
    uintptr_t get_base_addres(const char* libname);
    
    #ifdef __cplusplus
}
#endif

int hook_function(uintptr_t target, uintptr_t hook, uintptr_t* original, int count_args);

struct HookTarget
{
    const char* assembly;
    const char* namezpace;
    const char* klass;
    const char* method;

    int argsCount;

    void* handler;
    void** original;
};

void InstallDynamicHooks(std::vector<HookTarget> hooks);