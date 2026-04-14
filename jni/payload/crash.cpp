#include "crash.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

void crash_handler(int sig, siginfo_t* info, void* ucontext)
{
    FILE* f = fopen("/sdcard/Android/data/com.and.games505.TerrariaPaid/files/crash.txt", "w");
    if (!f) f = fopen("/sdcard/crash.txt", "w");

    if (f)
    {
        fprintf(f, "--- CRASH ---\n");
        fprintf(f, "Signal: %d\n", sig);
        fprintf(f, "Fault addr: %p\n", info->si_addr);

        ucontext_t* ctx = (ucontext_t*)ucontext;
        mcontext_t* mctx = &ctx->uc_mcontext;

        fprintf(f, "PC: 0x%llx\n", mctx->pc);
        fprintf(f, "SP: 0x%llx\n", mctx->sp);
        fprintf(f, "LR: 0x%llx\n", mctx->regs[30]);

        for (int i = 0; i < 10; ++i)
        {
            fprintf(f, "X%d: 0x%llx\n", i, mctx->regs[i]);
        }

        fprintf(f, "\n=== MAPS ===\n");
        FILE* maps = fopen("/proc/self/maps", "r");

        if (maps) 
        {
            char line[256];

            while (fgets(line, sizeof(line), maps)) 
            {
                fputs(line, f);
            }
            fclose(maps);
        }
        
        fclose(f);
    }

    signal(sig, SIG_DFL);
    raise(sig);
}

extern "C" __attribute__((visibility("default")))
void setup_crash_handler()
{
    struct sigaction sa;
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGILL,  &sa, NULL);
    sigaction(SIGBUS,  &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    
    LOGI("Crash handler installed");
}