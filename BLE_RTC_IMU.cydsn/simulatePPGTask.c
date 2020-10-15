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
#include "queue.h"
#include <stdio.h>
#include "rtcTask.h"

#define TIMER_PERIOD_MSEC   8U   /* Timer period in milliseconds */

struct SamplePPG{
    uint64_t time;
    uint32_t led_ir;
	uint32_t led_red;
	uint32_t led_green;
};

static int numSamplesPPG = 0;
QueueHandle_t simulatedPPGQueue;

/* These static functions are used by the IMU Task. These are not available 
   outside this file. See the respective function definitions for more 
   details */
void static PpgDataReady_Interrupt(void);
void static ppgStart(void);

void ppgStart(){
    
    /* Configure the ISR for the TCPWM peripheral*/
    Cy_SysInt_Init(&SysInt_TimerPPG_INT_cfg, PpgDataReady_Interrupt);

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

void ppgStop(){
    
}

void taskPPG(void *arg)
{
    (void)arg;
    
    printf("Starting PPG Read Task\r\n");
    simulatedPPGQueue = xQueueCreate(100, sizeof(struct SamplePPG));
    ppgStart();
        
    struct SamplePPG sample;
    
    while(1)
    {
        if(simulatedPPGQueue != 0){
            if(xQueueReceive(simulatedPPGQueue, (void*) &sample, (TickType_t) 5)){
                printf("PPG: {R:%lu G:%d IR:%d}, Time: %.0f, Count: %d\r\n",sample.led_red, sample.led_green, sample.led_ir,sample.time/1.0, numSamplesPPG);
            }
        }
    }
}

void static PpgDataReady_Interrupt(void)
{    
    /* Read the latest available data time and sample*/
    struct SamplePPG sample;
    sample.time = getCurrentTimeMillis();
    sample.led_green = numSamplesPPG % 16;
    sample.led_red = numSamplesPPG % 32;
    sample.led_ir = numSamplesPPG % 64;
    numSamplesPPG++;
    
    /* We have not woken a task at the start of the ISR. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* Clear any pending interrupts */
    Cy_TCPWM_ClearInterrupt(Timer_PPG_HW, Timer_PPG_CNT_NUM, CY_TCPWM_INT_ON_TC );
    NVIC_ClearPendingIRQ(SysInt_TimerPPG_INT_cfg.intrSrc);
    
    //printf("Queueing PPG Sample (%d %.0f)\r\n", numSamplesPPG, sample.time/1.0);
    
    /* Send imu sample to queue */
    xQueueSendFromISR(simulatedPPGQueue, (void *) &sample, &xHigherPriorityTaskWoken );
    
    /* Now the buffer is empty we can switch context if necessary. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    
}

/* [] END OF FILE */
