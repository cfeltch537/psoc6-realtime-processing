/******************************************************************************
* File Name: imu_task.c
*
* Version: 1.00
*
* Description: This file contains the task that handles imu sensing
*
* Related Document: CE218138_BLE_Thermometer_RTOS.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
/******************************************************************************
* This file contains the task that handles imu sensing 
*******************************************************************************/

/* Header file includes */
#include <math.h>
#include "motion_task.h"
#include "uart_debug.h"
#include "ble_task.h"
#include "task.h" 
#include "timers.h"

#define TIMER_PERIOD_MSEC   40U   /* Timer period in milliseconds */

struct SampleIMU{
    uint64_t time;
    uint32_t accX;
	uint32_t accY;
	uint32_t accZ;
    uint32_t gyroX;
	uint32_t gyroY;
	uint32_t gyroZ;
};

/* Queue handles used for imu commands and data */
QueueHandle_t imuCommandQ;
QueueHandle_t imuDataQ;

bool static processingComplete = false;
bool static sendIMUData = false;
uint32_t numSamplesIMU = 0;
uint64_t lastInterruptMillis;

/* Local methods*/
void static IMU_ISR(void);
void static IMUTimerStart(void);

/*******************************************************************************
* Function Name: void Task_IMU(void *pvParameters)   
********************************************************************************
* Summary:
*  Task that reads imu data from thermistor circuit  
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  void
*
*******************************************************************************/
void Task_IMU(void *pvParameters){ 
    /* Variable that stores commands received  */
    imu_command_t imuCommand;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Variable used to send commands and data to BLE task */
    ble_command_t bleCommandAndData;

    /* Remove warning for unused parameter */
    (void)pvParameters ;
    
    /* Start the timer that controls the processing interval */
    IMUTimerStart();                    
    
    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over 
           imuCommandQ */
        rtosApiResult = xQueueReceive(imuCommandQ, &imuCommand, portMAX_DELAY);
        
         /* Command has been received from imuCommandQ */ 
        if(rtosApiResult == pdTRUE)
        {   
            /* Take an action based on the command received */
            switch (imuCommand)
            {
                /* IMU data need to be sent */
                case SEND_IMU_START:
                    sendIMUData = true;
                    // START
                    break;
                 
                /* No imu data need to be sent */
                case SEND_IMU_STOP:
                    sendIMUData = false;
                    /* STOP */
                    break;
                /* Process imu data from CapSense widgets */
                case HANDLE_IMU_INTERRUPT:
                    processingComplete = false;
                    /* Read the latest available data time and sample*/
                    struct SampleIMU sample;
                    sample.time = 0; // getCurrentTimeMillis();
                    sample.accX = numSamplesIMU % 2;
                	sample.accY = numSamplesIMU % 4;
                	sample.accZ = numSamplesIMU % 8;
                    sample.gyroX = numSamplesIMU % 16;
                	sample.gyroY = numSamplesIMU % 32;
                	sample.gyroZ = numSamplesIMU % 64;
                    numSamplesIMU++;
                   
                    /* Send the processed imu data */
                    if(sendIMUData)
                    {
                        /* Pack the imu data, respective command and send 
                           to the queue */
                        bleCommandAndData.command = SEND_IMU_INDICATION;
                        bleCommandAndData.temperatureData = 5;
                        xQueueSend(bleCommandQ, &bleCommandAndData,0u);
                    }
                    else
                    {
                        /* Send imu sample to queue */
                        xQueueSend(imuDataQ, (void *) &sample, 0u);
                        processingComplete = true;
                    }
                    break;

                /* Invalid task notification value received */    
                default:
                    Task_DebugPrintf("Error!   : IMU - Invalid command "\
                                "received .Error Code:", imuCommand);
                    break;
            }
        }            
        /* Task has timed out and received no commands during an interval of 
        portMAXDELAY ticks */
        else
        {
            Task_DebugPrintf("Warning! : IMU - Task Timed out ", 0u);   
        }
    }
}

/*******************************************************************************
* Function Name: void isrADC(void)                         
********************************************************************************
* Summary:
*  Interrupt service routine of the Scanning SAR ADC
*
* Parameters:
*  None
*
* Return:
*  void
*
*******************************************************************************/
void static IMU_ISR(void)
{        
    /* Read the latest available data time and sample*/
    //lastInterruptMillis = getCurrentTimeMillis();
    lastInterruptMillis = 0;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Clear any pending interrupts */
    Cy_TCPWM_ClearInterrupt(Timer_IMU_HW, Timer_IMU_CNT_NUM, CY_TCPWM_INT_ON_TC );
    NVIC_ClearPendingIRQ(SysInt_TimerIMU_INT_cfg.intrSrc);
    
    /* Read interrupt status register to ensure write completed due to 
       buffered writes */
    (void)Cy_SAR_GetInterruptStatus(ADC_SAR__HW);
    
    /* Send command to process imu */
    imu_command_t imuCommand = HANDLE_IMU_INTERRUPT;
    rtosApiResult = xQueueSendFromISR(imuCommandQ, &imuCommand,0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdTRUE)
    {
        Task_DebugPrintf("Failure! : IMU  - Sending data to imu"\
                    "queue", 0u);    
    }  
}


/*******************************************************************************
* Function Name: void static IMUTimerStart(void)                  
********************************************************************************
* Summary:
*  This function starts the timer that provides timing to periodic
*  imu processing
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void static IMUTimerStart(void)
{
    /* Configure the ISR for the TCPWM peripheral*/
    Cy_SysInt_Init(&SysInt_TimerIMU_INT_cfg, IMU_ISR);

    /* Enable interrupt in NVIC */ 
    NVIC_EnableIRQ(SysInt_TimerIMU_INT_cfg.intrSrc);
    
    /* Start the TCPWM component in timer/counter mode. The return value of the
     * function indicates whether the arguments are valid or not. It is not used
     * here for simplicity. 
     */
    (void)Cy_TCPWM_Counter_Init(Timer_IMU_HW, Timer_IMU_CNT_NUM, &Timer_IMU_config); 
    Cy_TCPWM_Enable_Multiple(Timer_IMU_HW, Timer_IMU_CNT_MASK); /* Enable the counter instance */
    
    /* Set the timer period in milliseconds. To count N cycles, period should be
     * set to N-1.
     */
    Cy_TCPWM_Counter_SetPeriod(Timer_IMU_HW, Timer_IMU_CNT_NUM, TIMER_PERIOD_MSEC - 1);
    
    /* Trigger a software start on the counter instance. This is required when 
     * no other hardware input signal is connected to the component to act as
     * a trigger source. 
     */
    Cy_TCPWM_TriggerStart(Timer_IMU_HW, Timer_IMU_CNT_MASK);   
}

/* [] END OF FILE */
