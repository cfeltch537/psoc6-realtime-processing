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
#include "semphr.h"
#include <stdio.h>

/* Macros */
#define MAX_ATTEMPTS        (500u)  /* Maximum number of attempts for RTC operation */ 
#define INIT_DELAY          (5u)    /* delay 5 milliseconds before trying again */

#define SECONDS_PER_MIN     (60u)   /* used to keep values in range */
#define MINUTES_PER_HOUR    (60u)

#define TICK_INTERVAL       (1u)    /* seconds or minutes interval. The range should be 1-59 */
#define USE_SECONDS         (1u)    /* set to one to use, set to zero to not use */
#define USE_MINUTES         (0u)    /* use seconds OR minutes, not both  */

#define SYSTICK_MAX_CNT_BITS 24u  
#define SYSTICK_MAX_CNT (1<<SYSTICK_MAX_CNT_BITS)-1

/* The interrupt status variable */
static SemaphoreHandle_t rtcAlarmSemaphore;
static uint64_t unixTimeMillis = 1601054056807;
static uint32_t tickAtLastSecond = 0;
static uint32_t millis = 0;

/* 
    If an En field is set to CY_RTC_ALARM_DISABLE, the alarm
    function ignores the disabled field when looking for a match.
    Configured to ignore all matches but enable the alarm, and
    the alarm goes off once per second, because the RTC uses 
    one-second resolution. 
*/
cy_stc_rtc_alarm_t alarmConfig = 
{
    .sec            = RTC_INITIAL_DATE_SEC,
    .secEn          = CY_RTC_ALARM_DISABLE,
    .min            = RTC_INITIAL_DATE_MIN,
    .minEn          = CY_RTC_ALARM_DISABLE,
    .hour           = RTC_INITIAL_DATE_HOUR,
    .hourEn         = CY_RTC_ALARM_DISABLE,
    .dayOfWeek      = RTC_INITIAL_DATE_DOW,
    .dayOfWeekEn    = CY_RTC_ALARM_DISABLE,
    .date           = RTC_INITIAL_DATE_DOM,
    .dateEn         = CY_RTC_ALARM_DISABLE,
    .month          = RTC_INITIAL_DATE_MONTH,
    .monthEn        = CY_RTC_ALARM_DISABLE,
    .almEn          = CY_RTC_ALARM_ENABLE
};

/*******************************************************************************/
/*                      Function Prototypes                                    */
/*******************************************************************************/
cy_en_rtc_status_t RtcInit(void);
cy_en_rtc_status_t RtcAlarmConfig(void);
void RtcInterruptHandler(void);
void RtcStepAlarm(void);

// https://community.cypress.com/thread/53511

uint32_t getTick(void){
    /* Just for testing purposes */
    return Cy_SysTick_GetValue();
}

uint64_t getTimestamp(void){
    uint32_t sysTick = Cy_SysTick_GetValue();
    uint32_t ticks = tickAtLastSecond-sysTick;
    uint64_t millis = (uint64_t)ticks*1000/32768;
    return unixTimeMillis + millis;
}

void SysTickISRCallback(void)
{
    millis++;
}

/* timetask task */
void startTimekeeping(void)
{
    printf("Started Timekeeping\r\n");

    /* Init SysTick */
    /*Initialize and Enable the SysTick resource*/  
 
    //Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_LF, SYSTICK_MAX_CNT); // Do not use this. 
    

    
    //Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_IMO, 8000000/1000 -1);
    /* Find unused callback slot and assign the callback. */
    uint32_t i;
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (Cy_SysTick_GetCallback(i) == NULL)
        {
            /* Set callback */
            Cy_SysTick_SetCallback(i, SysTickISRCallback);
            break;
        }
    }
    
    /* Init RTC - Configure the time and date */
    if(RtcInit() != CY_RTC_SUCCESS)
    {
        /* If operation fails, halt */
        CY_ASSERT(0u);
    }
    else  /* Configures the alarm to enable interrupt */
    {   
        /*
            To create a periodic alarm once per minute, enable the seconds match.
            Do not enable the minutes match, because all that matters is the
            seconds number. If it's zero, then every time the RTC second wraps
            around to zero, there is a match, and the alarm goes off.
        */
        if( (TICK_INTERVAL == 1u) && (USE_MINUTES == 1u))
        {
            alarmConfig.secEn = CY_RTC_ALARM_ENABLE;
        }

        /* Now configure the alarm */
        if(RtcAlarmConfig() != CY_RTC_SUCCESS)
        {
           /* If operation fails, halt */
           CY_ASSERT(0u);
        }
        else
        {
	        /* This CE uses Alarm2, enable that interrupt */
		    Cy_RTC_SetInterruptMask(CY_RTC_INTR_ALARM2);
        }
    }
    
}

/* timetask task */
void timeTask(void *arg)
{
    (void)arg;
    //startTimekeeping();
        
    /* Create a semaphore. It will be set in the UART ISR when data is available */
    rtcAlarmSemaphore = xSemaphoreCreateBinary();   
    
    /* Enable RTC interrupt handler function */
    Cy_SysInt_Init(&RTC_RTC_IRQ_cfg, RtcInterruptHandler);
    NVIC_EnableIRQ(RTC_RTC_IRQ_cfg.intrSrc);
    
    while(1)
    {
        /* Wait here until the semaphore is given (i.e. set) by the ISR */
        xSemaphoreTake(rtcAlarmSemaphore,portMAX_DELAY);

    	/* A custom processing - toggle the LED (RED) */
    	Cy_GPIO_Inv(LED_R_0_PORT, LED_R_0_NUM);		
        
        cy_stc_rtc_config_t rtc_time;
        Cy_RTC_GetDateAndTime(&rtc_time);
        
        uint64 unixTimeMillis = getTimestamp();
        uint32 tick = getTick();
        
        //printf("Time Task: {Date: %04u/%02u/%02u, Time:%02u:%02u:%02u %, Millis:.0f\r\n",rtc_time.year, rtc_time.month, rtc_time.date, rtc_time.hour, rtc_time.min, rtc_time.sec, unixTimeMillis/1.0);
        printf("Time Task Tick: %u\r\n", millis);
    }
}


/******************************************************************************
* Function Name: RtcInit
*******************************************************************************
*
* Summary: 
*  This function configures the RTC registers.
*
* Parameters:
*  None
*
* Return:
*  cy_en_rtc_status_t
*       CY_RTC_SUCCESS      : Time and date configuration is successfully done.
*       CY_RTC_BAD_PARAM    : Date values are not valid.
*       CY_RTC_INVALID_STATE: RTC is busy state
*
******************************************************************************/
cy_en_rtc_status_t RtcInit(void)
{
    uint32_t attempts = MAX_ATTEMPTS;
    cy_en_rtc_status_t result;
    
    /* Setting the time and date can fail. For example the RTC might be busy.
       Check the result and try again, if necessary.  */
    do
    {
        result = Cy_RTC_Init(&RTC_config);
        attempts--;
        
        Cy_SysLib_Delay(INIT_DELAY);
    } while(( result != CY_RTC_SUCCESS) && (attempts != 0u));
    
	return (result);
}

/******************************************************************************
* Function Name: RtcAlarmConfig
*******************************************************************************
*
* Summary: 
*  This function configures the custom RTC alarm to utilize the custom tick
*  timer interrupt.
*
* Parameters:
*  None
*
* Return:
*  cy_en_rtc_status_t
*       CY_RTC_SUCCESS      : Time and date configuration is successfully done.
*       CY_RTC_BAD_PARAM    : Date values are not valid.
*       CY_RTC_INVALID_STATE: RTC is busy state
*
******************************************************************************/
cy_en_rtc_status_t RtcAlarmConfig(void)
{
    uint32_t attempts = MAX_ATTEMPTS;
    cy_en_rtc_status_t result;

    /* 
       Setting the alarm can fail. For example the RTC might be busy. 
       Check the result and try again, if necessary.
    */
    do
	{
		result = Cy_RTC_SetAlarmDateAndTime((cy_stc_rtc_alarm_t const *)&alarmConfig, CY_RTC_ALARM_2);
		attempts--;
        
		Cy_SysLib_Delay(INIT_DELAY);
    } while(( result != CY_RTC_SUCCESS) && (attempts != 0u));
    
	return (result);
}

/******************************************************************************
* Function Name: RtcInterruptHandler
*******************************************************************************
*
* Summary: 
*  This is the general RTC interrupt handler in CPU NVIC.
*  It calls the Alarm2 interrupt handler if that is the interrupt that occurs.
*
* Parameters:
*  None
*
* Return:
*  None
*
******************************************************************************/
void RtcInterruptHandler(void)
{
    /* No DST parameters are required for the custom tick. */
    Cy_RTC_Interrupt(NULL, false);
}

/******************************************************************************
* Function Name: Cy_RTC_Alarm2Interrupt
*******************************************************************************
*
* Summary: 
*  The function overrides the __WEAK Cy_RTC_Alarm2Interrupt() in cy_rtc.c to 
*  handle CY_RTC_ALARM_2 interrupt.
*
* Parameters:
*  None
*
* Return:
*  None
*
******************************************************************************/
void Cy_RTC_Alarm2Interrupt(void)
{
    /* the interrupt has fired, meaning time expired on the alarm and one second passed */
    unixTimeMillis+=1000;
    //Cy_SysTick_Clear();
    // If the semaphore causes a task switch you should yield to that task
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(rtcAlarmSemaphore,&xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken != pdFALSE)
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    
}

/* [] END OF FILE */

