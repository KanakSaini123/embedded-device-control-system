



/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "SensorData.h"
#include "SystemState.h"
#include "NetworkManager.h"



/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY       ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS         pdMS_TO_TICKS( 200UL )
#define mainTIMER_SEND_FREQUENCY_MS        pdMS_TO_TICKS( 2000UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH                   ( 2 )

/* The values sent to the queue receive task from the queue send task and the
 * queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK           ( 100UL )
#define mainVALUE_SENT_FROM_TIMER          ( 200UL )

/* This demo allows for users to perform actions with the keyboard. */
#define mainNO_KEY_PRESS_VALUE             ( -1 )
#define mainRESET_TIMER_KEY                ( 'r' )

/*-----------------------------------------------------------*/

static void SensorTask( void * pvParameters );   // SensorTask
static void ControlTask(void* pvParameters);
static void NetworkTask(void* pvParameters);
static const char* getStateName(SystemState state);
static void StatusTask(void* pvParameters);
static SensorData sensorData =
{
    24.0f,
    3.0f,
    40.0f
};
static SystemState systemState = STATE_STARTUP;
static volatile int newSensorData = 0;
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );


/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
// static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_blinky( void )
{
    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    printf( "\r\nStarting the blinky demo. Press \'%c\' to reset the software timer used in this demo.\r\n\r\n", mainRESET_TIMER_KEY );

    /* Create the queue. */
        xTaskCreate( 
            SensorTask, 
            "TX", 
            configMINIMAL_STACK_SIZE, 
            NULL, 
            mainQUEUE_SEND_TASK_PRIORITY, 
            NULL 
        );
        xTaskCreate(
            ControlTask,
            "Control",
            configMINIMAL_STACK_SIZE,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL
        );
        xTaskCreate(
            StatusTask,
            "StatusTask",
            configMINIMAL_STACK_SIZE,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL
        );
        xTaskCreate(
            NetworkTask,
            "NetworkTask",
            configMINIMAL_STACK_SIZE,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL
        );
        /* Create the software timer, but don't start it yet. */
        xTimer = xTimerCreate( "Timer",                     /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
                               xTimerPeriod,                /* The period of the software timer in ticks. */
                               pdTRUE,                      /* xAutoReload is set to pdTRUE, so this timer goes off periodically with a period of xTimerPeriod ticks. */
                               NULL,                        /* The timer's ID is not used. */
                               prvQueueSendTimerCallback ); /* The function executed when the timer expires. */

        xTimerStart( xTimer, 0 );                           /* The scheduler has not started so use a block time of 0. */

        /* Start the tasks and timer running. */
        vTaskStartScheduler();


    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/
static void NetworkTask(void* pvParameters)
{
    char buffer[512];

    (void)pvParameters;

    printf("[NETWORK] Starting network task...\r\n");

    if (NetworkManager_Initialize() == 0)
    {
        printf("[NETWORK] Initialization failed.\r\n");

        for (; ; )
        {
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    for (; ; )
    {
        if (NetworkManager_WaitForClient() == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        for (; ; )
        {
            int bytesReceived;

            bytesReceived = NetworkManager_Receive(
                buffer,
                sizeof(buffer)
            );

            if (bytesReceived <= 0)
            {
                printf("[NETWORK] Python client disconnected.\r\n");
                break;
            }

            printf(
                "[NETWORK] Received: %s\r\n",
                buffer
            );

            if (NetworkManager_ParseSensorData(
                buffer,
                &sensorData))
            {
                printf(
                    "[NETWORK] SensorData updated: %.1f V | %.1f A | %.1f C\r\n",
                    sensorData.voltage,
                    sensorData.current,
                    sensorData.temperature
                );

                newSensorData = 1;
            }
        }
    }
}

static const char* getStateName(SystemState state)
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

static void StatusTask(void* pvParameters)
{
    (void)pvParameters;

    for (; ; )
    {
        printf(
            "[STATUS] System running | Temperature: %.1f C | State: %s\r\n",
            sensorData.temperature,
            getStateName(systemState)
        );

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void ControlTask(void* pvParameters)
{
    (void)pvParameters;

    for (; ; )
    {
        if (newSensorData == 1)
        {
            if (sensorData.temperature >= 80.0f ||
                sensorData.current >= 9.0f ||
                sensorData.voltage >= 31.0f)
            {
                systemState = STATE_FAULT;
            }
            else if (sensorData.temperature >= 60.0f ||
                sensorData.current >= 5.0f ||
                sensorData.voltage >= 26.0f)
            {
                systemState = STATE_WARNING;
            }
            else
            {
                systemState = STATE_NORMAL;
            }

            printf(
                "[CONTROL] State: %s\r\n",
                getStateName(systemState)
            );

            NetworkManager_SendState(systemState);

            newSensorData = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


static void SensorTask( void * pvParameters )
{
        (void)pvParameters;

        for (; ; )
        {
            printf(
                "[SENSOR] Voltage: %.1f V | Current: %.1f A | Temperature: %.1f C\r\n",
                sensorData.voltage,
                sensorData.current,
                sensorData.temperature
            );

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
/*-----------------------------------------------------------*/

static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;

    /* This is the software timer callback function.  The software timer has a
     * period of two seconds and is reset each time a key is pressed.  This
     * callback function will execute if the timer expires, which will only happen
     * if a key is not pressed for two seconds. */

    /* Avoid compiler warnings resulting from the unused parameter. */
    ( void ) xTimerHandle;

    /* Send to the queue - causing the queue receive task to unblock and
     * write out a message.  This function is called from the timer/daemon task, so
     * must not block.  Hence the block time is set to 0. */
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* Called from prvKeyboardInterruptSimulatorTask(), which is defined in main.c. */
void vBlinkyKeyboardInterruptHandler( int xKeyPressed )
{
    /* Handle keyboard input. */
    switch( xKeyPressed )
    {
        case mainRESET_TIMER_KEY:

            if( xTimer != NULL )
            {
                /* Critical section around printf to prevent a deadlock
                 * on context switch. */
                taskENTER_CRITICAL();
                {
                    printf( "\r\nResetting software timer.\r\n\r\n" );
                }
                taskEXIT_CRITICAL();

                /* Reset the software timer. */
                xTimerReset( xTimer, portMAX_DELAY );
            }

            break;

        default:
            break;
    }
}
