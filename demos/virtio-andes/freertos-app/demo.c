/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* FreeRTOS kernel includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>

#include <uart.h>
#include <irq.h>
#include <plat.h>

#include <virtio_console.h>

/*
 * Prototypes for the standard FreeRTOS callback/hook functions implemented
 * within this file.  See https://www.freertos.org/a00016.html
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);
void vTask(void *pvParameters);

/*-----------------------------------------------------------*/

#define VIRTIO_CONSOLE_RX_IRQ_ID (22)
#define SHMEM_IRQ_ID (21)

/*
* The shared memory region defined.
*/
#ifndef SHMEM_BASE
#error SHMEM_BASE not defined
#endif

#ifndef IPC_BASE
#error IPC_BASE not defined
#endif

static char* const shmem_base = (char*)SHMEM_BASE;
static char* const freertos_message = (char*)IPC_BASE;
static char* const linux_message = (char*)(IPC_BASE + 0x2000);
static const size_t shmem_channel_size = 0x2000;
static const long mmio_base = 0xa003e00;

static struct virtio_console console;
static volatile int start_requested;

static int linux_requested_start(void)
{
    return strncmp(linux_message, "start", 5) == 0;
}

static void backend_retry_delay(void)
{
    for (volatile unsigned long outer = 0; outer < 10; outer++) {
        for (volatile unsigned long inner = 0; inner < 10000000UL; inner++) {
            __asm volatile("nop");
        }
    }
}

void virtio_console_rx_handler(unsigned id) {
    (void)id;

    if (virtio_console_receive(&console)) {
        virtio_console_rx_print_buffer(&console);
    }
}

void virtio_init(void)
{
    printf("Initializing virtio console ...\n");

    while (!virtio_console_init(&console, shmem_base, mmio_base)) {
        console.ready = false;
        printf("virtio console initialization failed, retrying ...\n");
        backend_retry_delay();
    }

    printf("virtio console initialized\n");

    irq_set_handler(VIRTIO_CONSOLE_RX_IRQ_ID, virtio_console_rx_handler);
    irq_set_prio(VIRTIO_CONSOLE_RX_IRQ_ID, IRQ_MAX_PRIO);
    irq_enable(VIRTIO_CONSOLE_RX_IRQ_ID);
}

void shmem_update_msg(unsigned long counter)
{
    sprintf(freertos_message, "freertos virtio-andes counter: %lu\n", counter);
}

void shmem_handler(unsigned id)
{
    (void)id;

    linux_message[shmem_channel_size - 1] = '\0';

    char *end = strchr(linux_message, '\n');
    if (end != NULL) {
        *end = '\0';
    }

    printf("message from linux: %s\n", linux_message);

    if (linux_requested_start()) {
        start_requested = 1;
        sprintf(freertos_message, "freertos start accepted\n");
    }
}

void shmem_init(void)
{
    memset(freertos_message, 0, shmem_channel_size);
    memset(linux_message, 0, shmem_channel_size);
    shmem_update_msg(0);

    irq_set_handler(SHMEM_IRQ_ID, shmem_handler);
    irq_set_prio(SHMEM_IRQ_ID, IRQ_MAX_PRIO);
    irq_enable(SHMEM_IRQ_ID);
}

void wait_for_linux_start(void)
{
    printf("Waiting for Linux start command on /dev/baoipc0 ...\n");
    sprintf(freertos_message, "freertos waiting for start\n");

    while (!start_requested) {
        if (linux_requested_start()) {
            start_requested = 1;
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    printf("Linux requested FreeRTOS start\n");
}

void vStartTask(void *pvParameters)
{
    (void)pvParameters;

    //wait_for_linux_start();
    //virtio_init();

    xTaskCreate(
        vTask,
        "Task1",
        configMINIMAL_STACK_SIZE,
        (void *)1,
        tskIDLE_PRIORITY + 1,
        NULL);

    xTaskCreate(
        vTask,
        "Task2",
        configMINIMAL_STACK_SIZE,
        (void *)2,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskDelete(NULL);
}

void vTask(void *pvParameters)
{
    unsigned long counter = 0;
    unsigned long id = (unsigned long)pvParameters;
    char msg[32] = {0};
    while (1)
    {
        sprintf(msg, "Task%d: %d\r\n", id, counter++);
        shmem_update_msg(counter);
        //virtio_console_transmit(&console, msg);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

int main(void){

    printf("Bao FreeRTOS guest - Andes AE350\n");

    shmem_init();

    xTaskCreate(
        vStartTask,
        "Start",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    vTaskStartScheduler();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
}
/*-----------------------------------------------------------*/

void vAssertCalled(void)
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    taskDISABLE_INTERRUPTS();
    while (ulSetTo1ToExitFunction != 1)
    {
        __asm volatile("NOP");
    }
}
/*-----------------------------------------------------------*/

/* This version of vApplicationAssert() is declared as a weak symbol to allow it
to be overridden by a version implemented within the application that is using
this BSP. */
void vApplicationAssert( const char *pcFileName, uint32_t ulLine )
{
volatile uint32_t ul = 0;
volatile const char *pcLocalFileName = pcFileName; /* To prevent pcFileName being optimized away. */
volatile uint32_t ulLocalLine = ulLine; /* To prevent ulLine being optimized away. */

	/* Prevent compile warnings about the following two variables being set but
	not referenced.  They are intended for viewing in the debugger. */
	( void ) pcLocalFileName;
	( void ) ulLocalLine;

	printf( "Assert failed in file %s, line %lu\r\n", pcLocalFileName, ulLocalLine );

	/* If this function is entered then a call to configASSERT() failed in the
	FreeRTOS code because of a fatal error.  The pcFileName and ulLine
	parameters hold the file name and line number in that file of the assert
	that failed.  Additionally, if using the debugger, the function call stack
	can be viewed to find which line failed its configASSERT() test.  Finally,
	the debugger can be used to set ul to a non-zero value, then step out of
	this function to find where the assert function was entered. */
	taskENTER_CRITICAL();
	{
		while( ul == 0 )
		{
			__asm volatile( "NOP" );
		}
	}
	taskEXIT_CRITICAL();
}
