#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef void (*t_SendChat)(void* chatMessage);
extern t_SendChat orig_SendChat;

void my_SendChat(void* chatMessage);