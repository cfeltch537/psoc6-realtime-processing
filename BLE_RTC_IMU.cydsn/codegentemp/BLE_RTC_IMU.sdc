# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\codyg\git\PSoC6\BLE_RTC_IMU.cydsn\BLE_RTC_IMU.cyprj
# Date: Wed, 23 Sep 2020 18:05:06 GMT
#set_units -time ns
create_clock -name {CyWCO} -period 30517.578125 -waveform {0 15258.7890625} [list [get_pins {ClockBlock/wco}]]
create_clock -name {CyILO} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/ilo}]]
create_clock -name {CyClk_LF} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/lfclk}]]
create_clock -name {CyFLL} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/fll}]]
create_clock -name {CyClk_HF0} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/hfclk_0}]]
create_clock -name {CyClk_Fast} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/fastclk}]]
create_clock -name {CyClk_Peri} -period 20 -waveform {0 10} [list [get_pins {ClockBlock/periclk}]]
create_generated_clock -name {CyClk_Slow} -source [get_pins {ClockBlock/periclk}] -edges {1 2 3} [list [get_pins {ClockBlock/slowclk}]]
create_generated_clock -name {I2C_1_SCBCLK} -source [get_pins {ClockBlock/periclk}] -edges {1 33 65} [list [get_pins {ClockBlock/ff_div_3}]]
create_generated_clock -name {UART_1_SCBCLK} -source [get_pins {ClockBlock/periclk}] -edges {1 37 73} [list [get_pins {ClockBlock/ff_div_5}]]
create_clock -name {CyPeriClk_App} -period 20 -waveform {0 10} [list [get_pins {ClockBlock/periclk_App}]]
create_clock -name {CyIMO} -period 125 -waveform {0 62.5} [list [get_pins {ClockBlock/imo}]]


# Component constraints for C:\Users\codyg\git\PSoC6\BLE_RTC_IMU.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\codyg\git\PSoC6\BLE_RTC_IMU.cydsn\BLE_RTC_IMU.cyprj
# Date: Wed, 23 Sep 2020 18:04:51 GMT
