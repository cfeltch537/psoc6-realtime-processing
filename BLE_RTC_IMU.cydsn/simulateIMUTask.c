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

static int numSamplesIMU = 0;
QueueHandle_t simulatedIMUQueue;
QueueHandle_t dataframeIMUQueue;

/* These static functions are used by the IMU Task. These are not available 
   outside this file. See the respective function definitions for more 
   details */
void static IMU_ISR(void);
void static startIMU(void);
struct _data_frame_t static sampleToDataFrame(struct SampleIMU);

void startIMU(){
    
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

void imuStop(){
    
}

void taskIMU(void *arg)
{
    (void)arg;
    
    printf("Starting IMU Read Task\r\n");
    simulatedIMUQueue = xQueueCreate(100, sizeof(struct SampleIMU));
    dataframeIMUQueue = xQueueCreate(100, sizeof(struct _data_frame_t));
    startIMU();
        
    struct SampleIMU sample;
    
    while(1)
    {
        if(simulatedIMUQueue != 0){
            if(xQueueReceive(simulatedIMUQueue, (void*) &sample, (TickType_t) 5)){
                // Print sample data to UART
                printf("Accel: {x:%lu y:%lu z:%lu}, Gyro: {x:%lu y:%lu z:%lu}, Time: %.0f, Steps: %d\r\n",
                    sample.accX,sample.accY,sample.accZ,sample.gyroX,sample.accY,sample.accZ,sample.time/1.0, numSamplesIMU);   
                // Convert Sample to DataFrame
                struct _data_frame_t dataframe = sampleToDataFrame(sample);
                xQueueSend(dataframeIMUQueue, (void *) &dataframe, (TickType_t) 5); // portMAX_DELAY
            }
        }
    }
}

void static IMU_ISR(void)
{    
    /* Read the latest available data time and sample*/
    struct SampleIMU sample;
    sample.time = getCurrentTimeMillis();
    sample.accX = numSamplesIMU % 2;
	sample.accY = numSamplesIMU % 4;
	sample.accZ = numSamplesIMU % 8;
    sample.gyroX = numSamplesIMU % 16;
	sample.gyroY = numSamplesIMU % 32;
	sample.gyroZ = numSamplesIMU % 64;
    numSamplesIMU++;
    
    /* We have not woken a task at the start of the ISR. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* Clear any pending interrupts */
    Cy_TCPWM_ClearInterrupt(Timer_IMU_HW, Timer_IMU_CNT_NUM, CY_TCPWM_INT_ON_TC );
    NVIC_ClearPendingIRQ(SysInt_TimerIMU_INT_cfg.intrSrc);
    
    //printf("Queueing IMU Sample (%d %.0f)\r\n", numSamplesIMU, sample.time/1.0);
    
    /* Send imu sample to queue */
    xQueueSendFromISR(simulatedIMUQueue, (void *) &sample, &xHigherPriorityTaskWoken );
    
    /* Now the buffer is empty we can switch context if necessary. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    
}

struct _data_frame_t static sampleToDataFrame(struct SampleIMU sample){
    
    struct _data_frame_t dataframe;
    
    /* Control fields */
    dataframe.sensor_status = 0;
    dataframe.battery_status = 0;
    dataframe.field_hdr = FRAME_FIELD_TIME | FRAME_FIELD_CAP | FRAME_FIELD_ACCEL | FRAME_FIELD_GYRO;
    dataframe.seq = numSamplesIMU;
    dataframe.session = 0;

    /* Data fields */
    dataframe.time_sec = sample.time/1000;
    dataframe.time_msec = sample.time%1000;
    dataframe.last_tick_usec = 0;
    dataframe.fuel = 99;
    dataframe.cap[0] = 1;
    dataframe.cap[1] = 2;
    dataframe.cap[2] = 3;
    dataframe.accel[0] = sample.accX;
    dataframe.accel[1] = sample.accY;
    dataframe.accel[2] = sample.accZ;
    dataframe.gyro[0] = sample.gyroX;
    dataframe.gyro[1] = sample.gyroY;
    dataframe.gyro[2] = sample.gyroZ;

    /* Sentinel Fields */
    /*
    dataframe.led_ir;
    dataframe.led_red;
    dataframe.led_green;
    dataframe.heart_rate;
    dataframe.blood_oxygen_level;
    dataframe.confidence_level;
    dataframe.finger_detected_status;
    dataframe.temperature;
    dataframe.sync;
    */

    return dataframe;
}

/* [] END OF FILE */
