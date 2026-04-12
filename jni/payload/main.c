#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>

#define TAG "Payload"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

typedef int (*il2cpp_init_fn)(const char* domain_name);
il2cpp_init_fn original_il2cpp_init = NULL;

int my_il2cpp_init(const char* domain_name) 
{
    LOGI("hook: before original call");
    int result = ((il2cpp_init_fn)original_il2cpp_init)(domain_name);
    LOGI("hook: after original call, result=%d", result);
    return result;
}

typedef double (*Player_Hurt_fn)(void* self, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable);
Player_Hurt_fn original_Player_Hurt = NULL;

double my_Player_Hurt(void* self, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable)
{
    LOGI("Player.Hurt called! damage=%d hitDirection=%d crit=%d", damage, hitDirection, crit);

    damage = 0;

    double result = original_Player_Hurt(self, damageSource, damage, hitDirection, pvp, quiet, crit, cooldownCounter, dodgeable);
    LOGI("Player.Hurt result=%f", result);
    return result;
}


void parse_maps()
{
    FILE* f = fopen("/proc/self/maps", "r");
    char line[512];

    while (fgets(line, sizeof(line), f))
    {
        if (strstr(line, "libil2cpp.so"))
        {
            LOGI("Found libil2cpp.so: %s", line);
        }
    }

    fclose(f);
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

static int is_adrp(uint32_t instr)
{
    return (instr & 0x9F000000) == 0x90000000;
}

static uintptr_t calc_adrp_target(uintptr_t pc, uint32_t instr)
{
    int64_t immlo = (instr >> 29) & 0x3;
    int64_t immhi = (instr >> 5) & 0x7FFFF;
    int64_t imm   = (immhi << 2) | immlo;

    imm = (imm << 43) >> 43;

    return (pc & ~0xFFF) + (imm << 12);
}

static int get_adrp_reg(uint32_t instr)
{
    return instr & 0x1F;
}

static int write_abs_load(uint32_t* dst, int reg, uintptr_t addr) 
{
    dst[0] = 0xD2800000 | ((addr & 0xFFFF) << 5) | reg;
    dst[1] = 0xF2A00000 | (((addr >> 16) & 0xFFFF) << 5) | reg;
    dst[2] = 0xF2C00000 | (((addr >> 32) & 0xFFFF) << 5) | reg;
    dst[3] = 0xF2E00000 | (((addr >> 48) & 0xFFFF) << 5) | reg;

    return 4; 
}

int hook_function(uintptr_t target, uintptr_t hook, uintptr_t* original)
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
    while (copied < 14)
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

void payload_init()
{
    LOGI("Payload API init!");

    uintptr_t lib_addr = get_base_addres("libil2cpp.so");
    LOGI("libil2cpp.so addr: 0x%lx", lib_addr);

    uintptr_t hurt_addr = lib_addr + 0x129F0D4;
    uintptr_t original = 0;

    LOGI("Player.Hurt addr: 0x%lx", hurt_addr);

    hook_function(hurt_addr, (uintptr_t)my_Player_Hurt, &original);
    original_Player_Hurt = (Player_Hurt_fn)original;
    
    LOGI("Done, waiting for il2cpp_init call...");
}