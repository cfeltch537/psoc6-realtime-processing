/******************************************************************************
* File Name: ppg_task.c
*
* Version: 1.00
*
* Description: This file contains the task that handles ppg sensing
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
* This file contains the task that handles ppg sensing 
*******************************************************************************/

/* Header file includes */
#include <math.h>
#include "ppg_task.h"
#include "uart_debug.h"
#include "ble_task.h"
#include "task.h" 
#include "timers.h"
#include "frame.h"

#define TIMER_PERIOD_MSEC   8U   /* Timer period in milliseconds */

/* Queue handles used for ppg commands and data */
QueueHandle_t ppgCommandQ;
QueueHandle_t ppgDataQ;

bool static processingComplete = false;

/* Bool for PPG Write State */
bool static indicateDataPPG = false;
bool static notifyDataPPG = false;

uint32_t numSamplesPPG = 0;
uint64_t lastInterruptMillis;

/* Local methods*/
void static PPG_ISR(void);
void static PPGTimerStart(void);

/*******************************************************************************
* Function Name: void Task_PPG(void *pvParameters)   
********************************************************************************
* Summary:
*  Task that reads ppg data from thermistor circuit  
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)                            
*
* Return:
*  void
*
*******************************************************************************/
void Task_PPG(void *pvParameters){ 
    /* Variable that stores commands received  */
    ppg_command_t ppgCommand;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Variable used to send commands and data to BLE task */
    ble_command_t bleCommandAndData;

    /* Remove warning for unused parameter */
    (void)pvParameters ;
    
    /* Start the timer that controls the processing interval */
    PPGTimerStart();                    
    
    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over 
           ppgCommandQ */
        rtosApiResult = xQueueReceive(ppgCommandQ, &ppgCommand, portMAX_DELAY);
        
         /* Command has been received from ppgCommandQ */ 
        if(rtosApiResult == pdTRUE)
        {   
            /* Take an action based on the command received */
            switch (ppgCommand)
            {
                /* PPG data need to be sent */
                case SEND_PPG_START:
                    Task_DebugPrintf("Info     : PPG - Starting PPG Sampling ", 0u);
                    notifyDataPPG = true;
                    break;
                /* No ppg data need to be sent */
                case SEND_PPG_STOP:
                    notifyDataPPG = false;
                    break;
                /* Process ppg data from CapSense widgets */
                case HANDLE_PPG_INTERRUPT:
                    {
                        struct _data_frame_t dataframe;
                        
                        /* Control fields */
                        dataframe.sensor_status = 0;
                        dataframe.battery_status = 0;
                        dataframe.field_hdr = FRAME_FIELD_TIME | FRAME_FIELD_SENTINEL;
                        dataframe.seq = numSamplesPPG;
                        dataframe.session = 0;

                        /* Data fields */
                        // FRAME_FIELD_TIME
                        dataframe.time_sec = lastInterruptMillis/1000;
                        dataframe.time_msec = lastInterruptMillis%1000;
                        dataframe.last_tick_usec = 0;
                        dataframe.fuel = 99;
                        // Sentinel Fields
                        dataframe.led_ir                    = numSamplesPPG % 2; 
                        dataframe.led_red                   = numSamplesPPG % 4;   
                        dataframe.led_green                 = numSamplesPPG % 8;
                        dataframe.heart_rate                = numSamplesPPG % 2; 
                        dataframe.blood_oxygen_level        = numSamplesPPG % 2; 
                        dataframe.confidence_level          = numSamplesPPG % 2; 
                        dataframe.finger_detected_status    = numSamplesPPG % 2; 
                        dataframe.temperature               = numSamplesPPG % 2; 
                        dataframe.sync                      = numSamplesPPG % 2; 
                        
                        numSamplesPPG++;
                       
                        /* Send the processed ppg data */
                        if(notifyDataPPG)
                        {
                            /* Pack the ppg data, respective command and send 
                               to the queue */
                            bleCommandAndData.command = SEND_PPG_NOTIFICATION;
                            bleCommandAndData.dataframe = dataframe;
                            //xQueueSend(bleCommandQ, &bleCommandAndData, 0u);
                            if( xQueueSend( bleCommandQ, ( void * ) &bleCommandAndData, ( TickType_t ) 0 ) != pdPASS )
                            {
                                // Failed to post the message
                                Task_DebugPrintf("Warning!  : PPG - Failed to Queue PPG Sample", 0u);
                            }else{
                                Task_DebugPrintf("Info      : PPG - Queued PPG Sample ", 0u);
                            }
                        }
                        else
                        {
                            /* Send ppg sample to queue */
                            //xQueueSend(ppgDataQ, (void *) &sample, 0u);
                            processingComplete = true;
                        }
                    }
                    break;

                /* Invalid task notification value received */    
                default:
                    Task_DebugPrintf("Error!   : PPG - Invalid command "\
                                "received .Error Code:", ppgCommand);
                    break;
            }
        }            
        /* Task has timed out and received no commands during an interval of 
        portMAXDELAY ticks */
        else
        {
            Task_DebugPrintf("Warning! : PPG - Task Timed out ", 0u);   
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
void static PPG_ISR(void)
{        
    /* Read the latest available data time and sample*/
    //lastInterruptMillis = getCurrentTimeMillis();
    lastInterruptMillis = 0;
    
    /* Variable used to store the return values of RTOS APIs */
    BaseType_t rtosApiResult;
    
    /* Clear any pending interrupts */
    Cy_TCPWM_ClearInterrupt(Timer_PPG_HW, Timer_PPG_CNT_NUM, CY_TCPWM_INT_ON_TC );
    NVIC_ClearPendingIRQ(SysInt_TimerPPG_INT_cfg.intrSrc);
    
    /* Read interrupt status register to ensure write completed due to 
       buffered writes */
    (void)Cy_SAR_GetInterruptStatus(ADC_SAR__HW);
    
    /* Send command to process ppg */
    ppg_command_t ppgCommand = HANDLE_PPG_INTERRUPT;
    rtosApiResult = xQueueSendFromISR(ppgCommandQ, &ppgCommand,0u);
    
    /* Check if the operation has been successful */
    if(rtosApiResult != pdTRUE)
    {
        Task_DebugPrintf("Failure! : PPG  - Sending data to ppg"\
                    "queue", 0u);    
    }  
}


/*******************************************************************************
* Function Name: void static PPGTimerStart(void)                  
********************************************************************************
* Summary:
*  This function starts the timer that provides timing to periodic
*  ppg processing
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void static PPGTimerStart(void)
{
    /* Configure the ISR for the TCPWM peripheral*/
    Cy_SysInt_Init(&SysInt_TimerPPG_INT_cfg, PPG_ISR);

    /* Enable interrupt in NVIC */ 
    NVIC_EnableIRQ(SysInt_TimerPPG_INT_cfg.intrSrc);
    
    /* Start the TCPWM component in timer/counter mode. The return value of the
     * function indicates whether the arguments are valid or not. It is not used
     * here for simplicity. 
     */
    (void)Cy_TCPWM_Counter_Init(Timer_PPG_HW, Timer_PPG_CNT_NUM, &Timer_PPG_config); 
    Cy_TCPWM_Enable_Multiple(Timer_PPG_HW, Timer_PPG_CNT_MASK); /* Enable the counter instance */
    
    /* Set the timer period in milliseconds. To count N cycles, period should be
     * set to N-1.
     */
    Cy_TCPWM_Counter_SetPeriod(Timer_PPG_HW, Timer_PPG_CNT_NUM, TIMER_PERIOD_MSEC - 1);
    
    /* Trigger a software start on the counter instance. This is required when 
     * no other hardware input signal is connected to the component to act as
     * a trigger source. 
     */
    Cy_TCPWM_TriggerStart(Timer_PPG_HW, Timer_PPG_CNT_MASK);   
}

/* [] END OF FILE */
