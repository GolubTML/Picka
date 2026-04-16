#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef void (*t_ProcessIncomingMessage)(void* instance, void* chatMessage, int clientID);
extern t_ProcessIncomingMessage orig_ProcessIncomingMessage;

void my_ProcessIncomingMessage(void* instance, void* chatMessage, int clientID);