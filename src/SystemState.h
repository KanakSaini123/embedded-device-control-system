#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

typedef enum
{
    STATE_STARTUP,
    STATE_NORMAL,
    STATE_WARNING,
    STATE_FAULT
} SystemState;

#endif