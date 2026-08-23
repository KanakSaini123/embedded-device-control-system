#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "NetworkManager.h"
#include "cJSON/cJSON.h"

#pragma comment(lib, "Ws2_32.lib")

#define NETWORK_PORT 5000
#define NETWORK_BUFFER_SIZE 512

static SOCKET serverSocket = INVALID_SOCKET;
static SOCKET clientSocket = INVALID_SOCKET;

int NetworkManager_ParseSensorData(
    const char* message,
    SensorData* data
)
{
    cJSON* root;
    cJSON* voltage;
    cJSON* current;
    cJSON* temperature;

    root = cJSON_Parse(message);

    if (root == NULL)
    {
        printf("[JSON] Invalid JSON.\r\n");
        return 0;
    }

    voltage = cJSON_GetObjectItem(root, "voltage");
    current = cJSON_GetObjectItem(root, "current");
    temperature = cJSON_GetObjectItem(root, "temperature");

    if (voltage == NULL ||
        current == NULL ||
        temperature == NULL)
    {
        printf("[JSON] Missing sensor value.\r\n");

        cJSON_Delete(root);
        return 0;
    }

    data->voltage = (float)voltage->valuedouble;
    data->current = (float)current->valuedouble;
    data->temperature = (float)temperature->valuedouble;

    cJSON_Delete(root);

    return 1;
}

static const char* getStateNameForNetwork(SystemState state)
{
    switch (state)
    {
    case STATE_STARTUP:
        return "STARTUP";

    case STATE_NORMAL:
        return "NORMAL";

    case STATE_WARNING:
        return "WARNING";

    case STATE_FAULT:
        return "FAULT";

    default:
        return "UNKNOWN";
    }
}

int NetworkManager_SendState(SystemState state)
{
    cJSON* root;
    char* message;
    int result;

    root = cJSON_CreateObject();

    if (root == NULL)
    {
        return 0;
    }

    cJSON_AddStringToObject(
        root,
        "state",
        getStateNameForNetwork(state)
    );

    message = cJSON_PrintUnformatted(root);

    if (message == NULL)
    {
        cJSON_Delete(root);
        return 0;
    }

    char response[128];

    snprintf(
        response,
        sizeof(response),
        "%s\n",
        message
    );

    result = NetworkManager_Send(response);

    cJSON_free(message);
    cJSON_Delete(root);

    return result;
}

static void TestJsonParsing(void)
{
    const char* message =
        "{\"voltage\":24.0,\"current\":3.0,\"temperature\":70.0}";

    cJSON* root;
    cJSON* voltage;
    cJSON* current;
    cJSON* temperature;

    root = cJSON_Parse(message);

    if (root == NULL)
    {
        printf("[JSON] Parsing failed.\r\n");
        return;
    }

    voltage = cJSON_GetObjectItem(root, "voltage");
    current = cJSON_GetObjectItem(root, "current");
    temperature = cJSON_GetObjectItem(root, "temperature");

    printf(
        "[JSON] Voltage: %.1f | Current: %.1f | Temperature: %.1f\r\n",
        voltage->valuedouble,
        current->valuedouble,
        temperature->valuedouble
    );

    cJSON_Delete(root);
}

int NetworkManager_Initialize(void)
{
    //TestJsonParsing();
    WSADATA wsaData;
    struct sockaddr_in serverAddress;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[NETWORK] WSAStartup failed.\r\n");
        return 0;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET)
    {
        printf("[NETWORK] Socket creation failed.\r\n");
        WSACleanup();
        return 0;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(NETWORK_PORT);

    if (bind(
        serverSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("[NETWORK] Bind failed.\r\n");
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        printf("[NETWORK] Listen failed.\r\n");
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }

    printf("[NETWORK] Server started on port %d.\r\n", NETWORK_PORT);

    return 1;
}

int NetworkManager_WaitForClient(void)
{
    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket == INVALID_SOCKET)
    {
        printf("[NETWORK] Client connection failed.\r\n");
        return 0;
    }

    printf("[NETWORK] Python client connected.\r\n");

    return 1;
}

int NetworkManager_Receive(char* buffer, int bufferSize)
{
    int bytesReceived;
    printf("[NETWORK] Waiting for data...\r\n");
    bytesReceived = recv(
        clientSocket,
        buffer,
        bufferSize - 1,
        0
    );

    if (bytesReceived <= 0)
    {
        return 0;
    }

    buffer[bytesReceived] = '\0';

    return bytesReceived;
}

int NetworkManager_Send(const char* message)
{
    int messageLength;

    messageLength = (int)strlen(message);

    if (send(clientSocket, message, messageLength, 0) == SOCKET_ERROR)
    {
        return 0;
    }

    return 1;
}

void NetworkManager_Close(void)
{
    if (clientSocket != INVALID_SOCKET)
    {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }

    if (serverSocket != INVALID_SOCKET)
    {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }

    WSACleanup();
}