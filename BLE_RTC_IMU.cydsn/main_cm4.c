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
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "bmi160Task.h"
#include "rtcTask.h"
#include "simulatePPGTask.h"
#include "simulateIMUTask.h"


/******************************************************************************
* Function Name: main
*******************************************************************************
*
* Summary: This is the system entrance point for Cortex-M4.
* This function generates a custom tick time interrupt using RTC alarm function.
*
* Parameters:
*  None
*
* Return:
*  int
*
* Side Effects:
*  None  
*
******************************************************************************/
int main(void)
{

    /* Enable global interrupt */
    __enable_irq();

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_1_Start();
    
    rtcStart();
    
    xTaskCreate(rtcTask, "RTC Task", 400, 0, 1, 0);
    // xTaskCreate(bmi160Task, "IMU Task", 400, 0, 1, 0);
    xTaskCreate(taskPPG, "PPG Task", 400, 0, 1, 0);
    xTaskCreate(taskIMU, "IMU Task", 400, 0, 1, 0);
    
    vTaskStartScheduler();
    
    for(;;)
    {
        /* Place your application code here. */
    }
}



/* [] END OF FILE */
