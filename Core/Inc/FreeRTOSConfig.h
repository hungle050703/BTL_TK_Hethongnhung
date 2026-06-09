/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS Kernel Configuration for Fire Alarm System (STM32H5 Series)
 ******************************************************************************
 */
/* USER CODE END Header */

#ifndef __FREERTOS_CONFIG_H
#define __FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */

/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif
#ifndef CMSIS_device_header
#define CMSIS_device_header "stm32h5xx.h"
#endif /* CMSIS_device_header */

/*-------------------- STM32H5 specific defines -------------------*/
#define configENABLE_TRUSTZONE                   0
#define configRUN_FREERTOS_SECURE_ONLY           0

/* KICH HOAT FPU: Sua tu 0 thanh 1 de ho tro tinh toan float an toan */
#define configENABLE_FPU                         1
#define configENABLE_MPU                         0

#define configUSE_PREEMPTION                     1
#define configSUPPORT_STATIC_ALLOCATION          1
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configCPU_CLOCK_HZ                       ( SystemCoreClock )
#define configTICK_RATE_HZ                       ((TickType_t)1000)
#define configMAX_PRIORITIES                     ( 56 )
#define configMINIMAL_STACK_SIZE                 ((uint16_t)128)

/* NANG DUNG LUONG HEAP: Tu 8KB len 32KB de du thcap phat vung nho cho cac Task Stacklon */
#define configTOTAL_HEAP_SIZE                    ((size_t)32768)

#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0
#define configMAX_TASK_NAME_LEN                  ( 16 )
#define configUSE_TRACE_FACILITY                 1
#define configUSE_16_BIT_TICKS                   0
#define configUSE_MUTEXES                        1
#define configQUEUE_REGISTRY_SIZE                8
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
#define configUSE_TASK_NOTIFICATIONS             1
#define configHEAP_CLEAR_MEMORY_ON_FREE          0
#define configUSE_MINI_LIST_ITEM                 1
#define configUSE_SB_COMPLETED_CALLBACK          0
#define configKERNEL_PROVIDED_STATIC_MEMORY      1
#define configSTATS_BUFFER_MAX_LENGTH            0xFFFF
#define configENABLE_HEAP_PROTECTOR              0
#define configUSE_EVENT_GROUPS                   1
#define configUSE_STREAM_BUFFERS                 1
#define configCHECK_HANDLER_INSTALLATION         1
#define configVALIDATE_HEAP_BLOCK_POINTER        0

/* USER CODE BEGIN MESSAGE_BUFFER_LENGTH_TYPE */
#define configMESSAGE_BUFFER_LENGTH_TYPE         size_t
/* USER CODE END MESSAGE_BUFFER_LENGTH_TYPE */

#define configRUN_TIME_COUNTER_TYPE              size_t
#define configSTACK_DEPTH_TYPE                   size_t

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                    0
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                ( 2 )
#define configTIMER_QUEUE_LENGTH                 10
#define configTIMER_TASK_STACK_DEPTH             128

/* CMSIS-RTOS V2 flags */
#define configUSE_OS2_THREAD_SUSPEND_RESUME  1
#define configUSE_OS2_THREAD_ENUMERATE       1
#define configUSE_OS2_EVENTFLAGS_FROM_ISR    1
#define configUSE_OS2_THREAD_FLAGS           1
#define configUSE_OS2_TIMER                  1
#define configUSE_OS2_MUTEX                  1

/* Set the following definitions to 1 to include the API function */
#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskCleanUpResources        0
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_xTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTimerPendFunctionCall       1
#define INCLUDE_xQueueGetMutexHolder         1
#define INCLUDE_xSemaphoreGetMutexHolder     1
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define INCLUDE_xTaskGetCurrentTaskHandle    1
#define INCLUDE_eTaskGetState                1
#define INCLUDE_xTaskAbortDelay              1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 #define configPRIO_BITS         4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics */
/* USER CODE BEGIN 1 */
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}
/* USER CODE END 1 */

#define SysTick_Handler xPortSysTickHandler

/* USER CODE BEGIN Defines */
/* Section where parameter definitions can be added */
/* USER CODE END Defines */

#endif /* __FREERTOS_CONFIG_H */
