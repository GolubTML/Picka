#pragma once
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

void crash_handler(int sig, siginfo_t* info, void* ucontext);
void setup_crash_handler();

#ifdef __cplusplus
}
#endif