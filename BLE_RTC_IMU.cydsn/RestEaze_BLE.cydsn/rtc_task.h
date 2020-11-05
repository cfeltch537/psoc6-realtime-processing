/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#pragma once

/* Header file includes */ 
#include <stdio.h>
#include "project.h"
#include "FreeRTOS.h"
#include "queue.h"


/* List of BLE commands */
typedef enum
{
    SET_RTC_TIME,
    START_MS_TIMEKEEPING,
    STOP_MS_TIMEKEEPING
}   rtc_commands_list_t;

/* Data-type of BLE commands and data */
typedef struct
{   
    rtc_commands_list_t     command;
    uint64_t                timestamp;
}   rtc_command_t;

/* Handles for the Queues that contain temperature command and data */ 
extern QueueHandle_t rtcCommandQ;

uint64_t getCurrentTimeMillis();
void rtcStart();
void rtcStop();
void Task_RTC(void *arg);

/* [] END OF FILE */
