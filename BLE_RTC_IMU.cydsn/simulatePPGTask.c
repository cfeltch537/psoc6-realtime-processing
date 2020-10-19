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
#include "frame.h"

#define TIMER_PERIOD_MSEC   8U   /* Timer period in milliseconds */

struct SamplePPG{
    uint64_t time;
    uint32_t led_ir;
	uint32_t led_red;
	uint32_t led_green;
};

static int numSamplesPPG = 0;
QueueHandle_t simulatedPPGQueue;
QueueHandle_t dataframePPGQueue;

/* These static functions are used by the IMU Task. These are not available 
   outside this file. See the respective function definitions for more 
   details */
void static PpgDataReady_Interrupt(void);
void static ppgStart(void);
struct _data_frame_t static sampleToDataFrame(struct SamplePPG);

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
    
    simulatedPPGQueue = xQueueCreate(100, sizeof(struct SamplePPG));
    dataframePPGQueue = xQueueCreate(250, sizeof(struct _data_frame_t));
    
    printf("Starting PPG Read Task\r\n");
    ppgStart();
        
    struct SamplePPG sample;
    
    while(1)
    {
        if(simulatedPPGQueue != 0){
            if(xQueueReceive(simulatedPPGQueue, (void*) &sample, (TickType_t) 5)){
                // Print sample data to UART
                printf("PPG: {R:%lu G:%d IR:%d}, Time: %.0f, Count: %d\r\n",sample.led_red, sample.led_green, sample.led_ir,sample.time/1.0, numSamplesPPG);
                // Convert Sample to DataFrame
                struct _data_frame_t dataframe = sampleToDataFrame(sample);
                xQueueSend(dataframePPGQueue, (void *) &dataframe, (TickType_t) 5);
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

struct _data_frame_t static sampleToDataFrame(struct SamplePPG sample){
    
    struct _data_frame_t dataframe;
    
    /* Control fields */
    dataframe.sensor_status = 0;
    dataframe.battery_status = 0;
    dataframe.field_hdr = FRAME_FIELD_TIME | FRAME_FIELD_SENTINEL;
    dataframe.seq = numSamplesPPG;
    dataframe.session = 0;

    /* Data fields */
    dataframe.time_sec = sample.time/1000;
    dataframe.time_msec = sample.time%1000;
    dataframe.last_tick_usec = 0;
    dataframe.fuel = 99;

    /* Sentinel Fields */
    dataframe.led_ir = sample.led_ir;
    dataframe.led_red = sample.led_red;
    dataframe.led_green = sample.led_green;
    dataframe.heart_rate = 60;
    dataframe.blood_oxygen_level = 61;
    dataframe.confidence_level = 62;
    dataframe.finger_detected_status = 1;
    dataframe.temperature = 63;
    dataframe.sync = 64;

    return dataframe;
}

/* [] END OF FILE */
