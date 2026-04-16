#include "hook.h"
#include "../log.h"
#include "Il2Cpp/Il2CppAPI.h"
#include "Il2Cpp/Il2CppResolver.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_adrp(uint32_t instr)
{
    return (instr & 0x9F000000) == 0x90000000;
}

uintptr_t calc_adrp_target(uintptr_t pc, uint32_t instr)
{
    int64_t immlo = (instr >> 29) & 0x3;
    int64_t immhi = (instr >> 5) & 0x7FFFF;
    int64_t imm   = (immhi << 2) | immlo;

    imm = (imm << 43) >> 43;

    return (pc & ~0xFFF) + (imm << 12);
}

int get_adrp_reg(uint32_t instr)
{
    return instr & 0x1F;
}

int write_abs_load(uint32_t* dst, int reg, uintptr_t addr)
{
    dst[0] = 0xD2800000 | ((addr & 0xFFFF) << 5) | reg;
    dst[1] = 0xF2A00000 | (((addr >> 16) & 0xFFFF) << 5) | reg;
    dst[2] = 0xF2C00000 | (((addr >> 32) & 0xFFFF) << 5) | reg;
    dst[3] = 0xF2E00000 | (((addr >> 48) & 0xFFFF) << 5) | reg;

    return 4; 
}

int hook_function(uintptr_t target, uintptr_t hook, uintptr_t* original, int count_args)
{
    uint32_t* trampoline = (uint32_t*)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (trampoline == MAP_FAILED)
    {
        LOGI("mmap failed!");
        return -1;
    }

    uint32_t* src = (uint32_t*)target;
    int tramp_idx = 0; 
    int copied = 0;

    // here we need to change number for different methods. For il2cpp_init it's 5, for Player.Hurt it's 14
    while (copied < count_args)
    {
        uint32_t instr = src[copied];
        uintptr_t instr_pc = target + copied * 4;

        if (is_adrp(instr))
        {
            uintptr_t adrp_result = calc_adrp_target(instr_pc, instr);
            int reg = get_adrp_reg(instr);

            LOGI("Fixing adrp at offset %d: reg=x%d target=0x%lx", copied, reg, adrp_result);

            tramp_idx += write_abs_load(&trampoline[tramp_idx], reg, adrp_result);
        }
        else
        {
            trampoline[tramp_idx++] = instr;
        }

        if ((instr & 0xFFFFFC1F) == 0xD61F0000) 
        { 
            LOGI("Detected Branch to register at offset %d, copying as is but high risk!", copied);
        }

        copied++;
    }

    trampoline[tramp_idx++] = 0x58000050;
    trampoline[tramp_idx++] = 0xD61F0200;

    uintptr_t continue_addr = target + copied * 4;

    memcpy(&trampoline[tramp_idx], &continue_addr, 8);

    __builtin___clear_cache((char*)trampoline, (char*)trampoline + tramp_idx * 4 + 8);

    LOGI("Trampoline at: 0x%lx", (uintptr_t)trampoline);

    LOGI("Trampoline dump:");
    for (int i = 0; i < tramp_idx + 2; i++) 
    {
        LOGI("  [%d] 0x%08x", i, trampoline[i]);
    }
    LOGI("  continue_addr: 0x%lx", continue_addr);

    *original = (uintptr_t)trampoline;

    uintptr_t page = target & ~0xFFF;
    mprotect((void*)page, 0x2000, PROT_READ | PROT_WRITE | PROT_EXEC);

    uint32_t* patch_dst = (uint32_t*)target;

    patch_dst[0] = 0x58000050;
    patch_dst[1] = 0xD61F0200;

    memcpy(&patch_dst[2], &hook, 8);

    __builtin___clear_cache((char*)target, (char*)target + 12);

    mprotect((void*)page, 0x2000, PROT_READ | PROT_EXEC);

    LOGI("Hook installed at: 0x%lx → 0x%lx", target, hook);
    return 0;
}

uintptr_t get_base_addres(const char* libname)
{
    FILE* f = fopen("/proc/self/maps", "r");
    char line[512];

    while (fgets(line, sizeof(line), f))
    {
        if (strstr(line, libname) && strstr(line, "r-xp"))
        {
            fclose(f);
            return (uintptr_t)strtoull(line, NULL, 16);
        }
    }

    fclose(f);
    return 0;
}

void InstallDynamicHooks(std::vector<HookTarget> hooks)
{
    for (const auto& target: hooks)
    {
        void* method_info = IL2CPP::Resolver::FindMethod(
            target.assembly, target.namezpace,
            target.klass, target.method,
            target.argsCount
        );

        if (method_info)
        {
            uintptr_t func_addr = *(uintptr_t*)method_info;
            uintptr_t original_ptr  = 0;

            hook_function(func_addr, (uintptr_t)target.handler, &original_ptr, 4); // maybe 4 bytes for instructions bad, but it works with most of methods

            *target.original = (void*)original_ptr;
            LOGI("Hooked: %s.%s at %p", target.klass, target.method, (void*)func_addr);
        }
        else
        {
            LOGI("Failed to find: %s.%s", target.klass, target.method);
        }
    }
}