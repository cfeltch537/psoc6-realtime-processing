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
#include "bmi160.h"
#include "rtcTask.h"

struct imu_sample{
    struct bmi160_sensor_data acc;
    struct bmi160_sensor_data gyro;
    uint64_t time;
};

// 32768 = 2g
#define MAXACCEL (32768/2)
// 32768 = 125dps
#define MAXGYRO (32768/125)

static struct bmi160_dev bmi160Dev;
int numSamples = 0;
QueueHandle_t rawImuQueue;


/* These static functions are used by the IMU Task. These are not available 
   outside this file. See the respective function definitions for more 
   details */
void static ImuDataReady_Interrupt(void);
int8_t static MotionSensor_ConfigDataReadyIntr(void);

static int8_t BMI160BurstWrite(uint8_t dev_addr, uint8_t reg_addr,uint8_t *data, uint16_t len)
{  
    Cy_SCB_I2C_MasterSendStart(I2C_1_HW,dev_addr,CY_SCB_I2C_WRITE_XFER,0,&I2C_1_context);
    Cy_SCB_I2C_MasterWriteByte(I2C_1_HW,reg_addr,0,&I2C_1_context);
    for(int i = 0;i<len; i++)
    { 
        Cy_SCB_I2C_MasterWriteByte(I2C_1_HW,data[i],0,&I2C_1_context);
    }
    
    Cy_SCB_I2C_MasterSendStop(I2C_1_HW,0,&I2C_1_context);
    
    return 0;
}

// This function supports the BMP180 library and read I2C Registers
static int8_t BMI160BurstRead(uint8_t dev_addr, uint8_t reg_addr,uint8_t *data, uint16_t len)
{   
    Cy_SCB_I2C_MasterSendStart(I2C_1_HW,dev_addr,CY_SCB_I2C_WRITE_XFER,0,&I2C_1_context);
    Cy_SCB_I2C_MasterWriteByte(I2C_1_HW,reg_addr,0,&I2C_1_context);
    Cy_SCB_I2C_MasterSendReStart(I2C_1_HW,dev_addr,CY_SCB_I2C_READ_XFER,0,&I2C_1_context);
    for(int i = 0;i<len-1; i++)
    {
        Cy_SCB_I2C_MasterReadByte(I2C_1_HW,CY_SCB_I2C_ACK,&data[i],0,&I2C_1_context);
    }
    Cy_SCB_I2C_MasterReadByte(I2C_1_HW,CY_SCB_I2C_NAK,&data[len-1],0,&I2C_1_context);
    
    Cy_SCB_I2C_MasterSendStop(I2C_1_HW,0,&I2C_1_context);

    return 0;
}

static void bmi160Init(void)
{
    rawImuQueue = xQueueCreate(100, sizeof(struct imu_sample));
    vTaskDelay(100);

    /* BMI160 */
    bmi160Dev.read = (bmi160_com_fptr_t)BMI160BurstRead;
    bmi160Dev.write = (bmi160_com_fptr_t)BMI160BurstWrite;
    bmi160Dev.delay_ms = (bmi160_delay_fptr_t)vTaskDelay;
    bmi160Dev.id = BMI160_I2C_ADDR;  // I2C device address

    bmi160_init(&bmi160Dev); // initialize the device
  
    bmi160Dev.gyro_cfg.odr = BMI160_GYRO_ODR_25HZ;
    bmi160Dev.gyro_cfg.range = BMI160_GYRO_RANGE_125_DPS;
    bmi160Dev.gyro_cfg.bw = BMI160_GYRO_BW_OSR4_MODE;

    /* Select the power mode of Gyroscope sensor */
    bmi160Dev.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

    bmi160Dev.accel_cfg.odr = BMI160_ACCEL_ODR_25HZ;
    bmi160Dev.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
    bmi160Dev.accel_cfg.bw = BMI160_ACCEL_BW_OSR4_AVG1;
    
    /* Select the power mode of Accel sensor */
    bmi160Dev.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

    /* Configure Data Ready Interrupt */
    MotionSensor_ConfigDataReadyIntr();
    
    /* Set the sensor configuration */
    bmi160_set_sens_conf(&bmi160Dev);
    bmi160Dev.delay_ms(50);
    
}

void bmi160Task(void *arg)
{
    (void)arg;
    
    printf("Started IMU Read Task\r\n");   
    I2C_1_Start();
    bmi160Init();

    float gx,gy,gz,dpsx,dpsy,dpsz;
    struct imu_sample sample;
    
    while(1)
    {
    
        if(rawImuQueue != 0){
            if(xQueueReceive(rawImuQueue, (void*) &sample, (TickType_t) 5)){
                gx = (float)sample.acc.x/MAXACCEL;
                gy = (float)sample.acc.y/MAXACCEL;
                gz = (float)sample.acc.z/MAXACCEL;
                dpsx = (float)sample.gyro.x/MAXGYRO;
                dpsy = (float)sample.gyro.y/MAXGYRO;
                dpsz = (float)sample.gyro.z/MAXGYRO;
                printf("Accel: {x:%1.2f y:%1.2f z:%1.2f}, Gyro: {x:%1.2f y:%1.2f z:%1.2f}, Time: %.0f, Steps: %d\r\n",gx,gy,gz,dpsx,dpsy,dpsz,sample.time/1.0, numSamples);   
            }
        }
    }
}

void static ImuDataReady_Interrupt(void)
{    
    /* Read the latest available data time and sample*/
    struct imu_sample sample;
    sample.time = getCurrentTimeMillis();
    bmi160_get_sensor_data(BMI160_BOTH_ACCEL_AND_GYRO, &sample.acc, &sample.gyro, &bmi160Dev);
    numSamples++;
    
    /* We have not woken a task at the start of the ISR. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* Clear any pending interrupts */
    Cy_GPIO_ClearInterrupt(Pin_ImuDataReady_INT_PORT, Pin_ImuDataReady_INT_NUM);
    NVIC_ClearPendingIRQ(SysInt_ImuDataReadyINT_cfg.intrSrc);
    
    /* Send imu sample to queue */
    xQueueSendFromISR(rawImuQueue, (void *) &sample, &xHigherPriorityTaskWoken );
    
    /* Now the buffer is empty we can switch context if necessary. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken );
    
}

int8_t static MotionSensor_ConfigDataReadyIntr(void)
{
    struct bmi160_int_settg int_config;
    uint8_t rslt = BMI160_OK;
    uint8_t step_enable = 1;
    
    /* Map the step interrupt to INT1 pin */
    int_config.int_channel = BMI160_INT_CHANNEL_1;

    /* Select the Interrupt type Step Detector interrupt */
    int_config.int_type = BMI160_ACC_GYRO_DATA_RDY_INT;
    
    /* Interrupt pin configuration */

    /* Enabling interrupt pins to act as output pin */
    int_config.int_pin_settg.output_en = BMI160_ENABLE;
    /* Choosing push-pull mode for interrupt pin */
    int_config.int_pin_settg.output_mode = BMI160_DISABLE;
    /* Choosing active High output */
    int_config.int_pin_settg.output_type = BMI160_ENABLE;
    /* Choosing edge triggered output */
    int_config.int_pin_settg.edge_ctrl = BMI160_ENABLE;
    /* Disabling interrupt pin to act as input */
    int_config.int_pin_settg.input_en = BMI160_DISABLE;
    /* non-latched output */
    int_config.int_pin_settg.latch_dur =BMI160_LATCH_DUR_NONE;
    
    /* Set the Step Detector interrupt */
    rslt = bmi160_set_int_config(&int_config, &bmi160Dev);
      
    /* Initialize and enable Step Interrupt */
    Cy_SysInt_Init(&SysInt_ImuDataReadyINT_cfg, ImuDataReady_Interrupt);
    NVIC_EnableIRQ(SysInt_ImuDataReadyINT_cfg.intrSrc);
    
    return rslt;
}

/* [] END OF FILE */
