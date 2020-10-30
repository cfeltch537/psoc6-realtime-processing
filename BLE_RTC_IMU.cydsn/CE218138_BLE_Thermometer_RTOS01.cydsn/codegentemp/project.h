/*******************************************************************************
* File Name: project.h
* 
* PSoC Creator  4.2
*
* Description:
* It contains references to all generated header files and should not be modified.
* This file is automatically generated by PSoC Creator.
*
********************************************************************************
* Copyright (c) 2007-2018 Cypress Semiconductor.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/

#include "cy8c6347bzi_bld53.h"
#include "sysint/cy_sysint.h"
#include "ipc/cy_ipc_drv.h"
#include "ipc/cy_ipc_sema.h"
#include "ipc/cy_ipc_pipe.h"
#include "sysclk/cy_sysclk.h"
#include "wdt/cy_wdt.h"
#include "rtc/cy_rtc.h"
#include "tcpwm/cy_tcpwm.h"
#include "tcpwm/cy_tcpwm_counter.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "tcpwm/cy_tcpwm_quaddec.h"
#include "scb/cy_scb_common.h"
#include "scb/cy_scb_i2c.h"
#include "scb/cy_scb_ezi2c.h"
#include "scb/cy_scb_spi.h"
#include "scb/cy_scb_uart.h"
#include "efuse/cy_efuse.h"
#include "flash/cy_flash.h"
#include "gpio/cy_gpio.h"
#include "sar/cy_sar.h"
#include "sysanalog/cy_sysanalog.h"
#include "syspm/cy_syspm.h"
#include "syslib/cy_syslib.h"
#include "trigmux/cy_trigmux.h"
#include "systick/cy_systick.h"
#include "profile/cy_profile.h"
#include "prot/cy_prot.h"
#include "lvd/cy_lvd.h"
#include "ble/cy_ble.h"
#include "ble/cy_ble_hal_pvt.h"
#include "ble/cy_ble_defines.h"
#include "ble/cy_ble_gap.h"
#include "ble/cy_ble_gatt.h"
#include "ble/cy_ble_event_handler.h"
#include "ble/cy_ble_stack.h"
#include "ble/cy_ble_stack_host_main.h"
#include "ble/cy_ble_stack_gatt_server.h"
#include "ble/cy_ble_stack_gap_peripheral.h"
#include "ble/cy_ble_stack_gatt_client.h"
#include "ble/cy_ble_stack_l2cap.h"
#include "ble/cy_ble_stack_pvt.h"
#include "ble/cy_ble_stack_gatt_db.h"
#include "ble/cy_ble_stack_gap_central.h"
#include "ble/cy_ble_stack_gap.h"
#include "ble/cy_ble_stack_gatt.h"
#include "ble/cy_ble_stack_host_error.h"
#include "ble/cy_ble_ancs.h"
#include "ble/cy_ble_ans.h"
#include "ble/cy_ble_aios.h"
#include "ble/cy_ble_bas.h"
#include "ble/cy_ble_bcs.h"
#include "ble/cy_ble_bls.h"
#include "ble/cy_ble_bms.h"
#include "ble/cy_ble_bts.h"
#include "ble/cy_ble_cgms.h"
#include "ble/cy_ble_cps.h"
#include "ble/cy_ble_cscs.h"
#include "ble/cy_ble_cts.h"
#include "ble/cy_ble_custom.h"
#include "ble/cy_ble_dis.h"
#include "ble/cy_ble_ess.h"
#include "ble/cy_ble_gls.h"
#include "ble/cy_ble_hids.h"
#include "ble/cy_ble_hps.h"
#include "ble/cy_ble_hrs.h"
#include "ble/cy_ble_hts.h"
#include "ble/cy_ble_ias.h"
#include "ble/cy_ble_ips.h"
#include "ble/cy_ble_lls.h"
#include "ble/cy_ble_lns.h"
#include "ble/cy_ble_ndcs.h"
#include "ble/cy_ble_plxs.h"
#include "ble/cy_ble_pass.h"
#include "ble/cy_ble_rscs.h"
#include "ble/cy_ble_rtus.h"
#include "ble/cy_ble_scps.h"
#include "ble/cy_ble_tps.h"
#include "ble/cy_ble_uds.h"
#include "ble/cy_ble_wpts.h"
#include "ble/cy_ble_wss.h"
#include "ble/cy_ble_clk.h"
#include "psoc63_config.h"
#include "cy_device_headers.h"
#if ((__CORTEX_M == 0) || (__CORTEX_M == 4))
    #include "gpio_psoc63_116_bga_ble.h"
#endif /*(__CORTEX_M == 0) || (__CORTEX_M == 4)*/
#include "cyip_sflash.h"
#include "cyip_peri.h"
#include "cyip_crypto.h"
#include "cyip_cpuss.h"
#include "cyip_fault.h"
#include "cyip_ipc.h"
#include "cyip_prot.h"
#include "cyip_flashc.h"
#include "cyip_srss.h"
#include "cyip_backup.h"
#include "cyip_dw.h"
#include "cyip_efuse.h"
#include "cyip_efuse_data.h"
#include "cyip_profile.h"
#include "cyip_hsiom.h"
#include "cyip_gpio.h"
#include "cyip_smartio.h"
#include "cyip_udb.h"
#include "cyip_lpcomp.h"
#include "cyip_csd.h"
#include "cyip_tcpwm.h"
#include "cyip_lcd.h"
#include "cyip_ble.h"
#include "cyip_usbfs.h"
#include "cyip_smif.h"
#include "cyip_scb.h"
#include "cyip_ctbm.h"
#include "cyip_ctdac.h"
#include "cyip_sar.h"
#include "cyip_pass.h"
#include "cyip_i2s.h"
#include "cyip_pdm.h"
#include "cyip_headers.h"
#include "cyfitter_cfg.h"
#include "cyfitter_sysint_cfg.h"
#include "cydevice_trm.h"
#include "cyfitter.h"
#include "cyfitter_gpio.h"
#include "cyfitter_sysint.h"
#include "cydisabledsheets.h"
#include "BLE.h"
#include "BLE_config.h"
#include "CY_EINK_SPIM.h"
#include "ADC.h"
#include "DEBUG_UART.h"
#include "Clock_1khz.h"
#include "Counter_ms.h"
#include "Timer_PPG.h"
#include "Timer_IMU.h"
#include "RTC.h"
#include "CY_EINK_SPIM_SCBCLK.h"
#include "ADC_intSarClock.h"
#include "DEBUG_UART_SCBCLK.h"

/*[]*/

