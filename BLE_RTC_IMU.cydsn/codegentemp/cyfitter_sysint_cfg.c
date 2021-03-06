/*******************************************************************************
* File Name: cyfitter_sysint_cfg.c
* 
* PSoC Creator  4.2
*
* Description:
* 
* This file is automatically generated by PSoC Creator.
*
********************************************************************************
* Copyright (c) 2007-2018 Cypress Semiconductor.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/

#include "cyfitter_sysint.h"
#include "cyfitter_sysint_cfg.h"

/* ARM CM4 */
#if (((__CORTEX_M == 4) && (CY_CORE_ID == 0)))

    /* RTC_RTC_IRQ */
    const cy_stc_sysint_t RTC_RTC_IRQ_cfg = {
        .intrSrc = (IRQn_Type)RTC_RTC_IRQ__INTC_NUMBER,
        .intrPriority = RTC_RTC_IRQ__INTC_CORTEXM4_PRIORITY
    };

    /* I2C_1_SCB_IRQ */
    const cy_stc_sysint_t I2C_1_SCB_IRQ_cfg = {
        .intrSrc = (IRQn_Type)I2C_1_SCB_IRQ__INTC_NUMBER,
        .intrPriority = I2C_1_SCB_IRQ__INTC_CORTEXM4_PRIORITY
    };

    /* UART_1_SCB_IRQ */
    const cy_stc_sysint_t UART_1_SCB_IRQ_cfg = {
        .intrSrc = (IRQn_Type)UART_1_SCB_IRQ__INTC_NUMBER,
        .intrPriority = UART_1_SCB_IRQ__INTC_CORTEXM4_PRIORITY
    };

    /* SysInt_TimerPPG_INT */
    const cy_stc_sysint_t SysInt_TimerPPG_INT_cfg = {
        .intrSrc = (IRQn_Type)SysInt_TimerPPG_INT__INTC_NUMBER,
        .intrPriority = SysInt_TimerPPG_INT__INTC_CORTEXM4_PRIORITY
    };

    /* SysInt_TimerIMU_INT */
    const cy_stc_sysint_t SysInt_TimerIMU_INT_cfg = {
        .intrSrc = (IRQn_Type)SysInt_TimerIMU_INT__INTC_NUMBER,
        .intrPriority = SysInt_TimerIMU_INT__INTC_CORTEXM4_PRIORITY
    };

    /* SysInt_ImuDataReadyINT */
    const cy_stc_sysint_t SysInt_ImuDataReadyINT_cfg = {
        .intrSrc = (IRQn_Type)SysInt_ImuDataReadyINT__INTC_NUMBER,
        .intrPriority = SysInt_ImuDataReadyINT__INTC_CORTEXM4_PRIORITY
    };

#endif /* ((__CORTEX_M == 4) && (CY_CORE_ID == 0)) */

