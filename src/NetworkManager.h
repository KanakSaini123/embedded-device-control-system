#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include "SensorData.h"
#include "SystemState.h"

int NetworkManager_SendState( SystemState state );

int NetworkManager_Initialize(void);
int NetworkManager_WaitForClient(void);
int NetworkManager_Receive(char* buffer, int bufferSize);
int NetworkManager_Send(const char* message);
void NetworkManager_Close(void);

int NetworkManager_ParseSensorData(
    const char* message,
    SensorData* data
);
#endif
