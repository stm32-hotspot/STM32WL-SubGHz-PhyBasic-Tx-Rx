# STM32-Hotspot/STM32WL_SubGHz_Phy_Basic_Tx_Rx MCU Firmware Package

![latest tag](https://img.shields.io/github/v/tag/stm32-hotspot/STM32WL-SubGHz-PhyBasic-Tx-Rx.svg?color=brightgreen)
![based on](https://img.shields.io/badge/based%20on%20STM32CubeWL-v1.2.0-blue.svg)

## Example

This Hotspot FW package includes:
* Application example under "Projects\NUCLEO-WL55JC\Applications\SubGHz_Phy" called SubGHz_Phy_Basic_LoRa.     
   * This example implements a basic LoRa radio transmitter and receiver application between two STM32WL55xx devices (note, two NUCLEO-WL55JC1 are needed; for the Tx and Rx, respectively). The Tx device transmits a short 10 byte packet every 2s, and the Rx device receives these packets and displays the payload on the hyperterminal. 
   * The example also aims to demonstrate to the user how to configure the system (pinout, clocks, peripherals, utilities) and SubGhz middleware from STM32CubeMX to enable the radio in LoRa modulation mode, and show how to schedule and enable a transmission and receive operation from the firmware application.       
   * Development tools, toolchains/compilers: STM32CubeMX v6.5.0, IAR EWARM V9.20.4, STM32CubeIDE v1.8.0
   * Supported Devices and hardware boards: NUCLEO-WL55JC1
   * Known limitations: None

This Hotspot FW package includes:
* Application example under "Projects\NUCLEO-WL55JC\Applications\SubGHz_Phy" called SubGHz_Phy_Basic_FSK.     
   * This example implements a basic FSK radio transmitter and receiver application between two STM32WL55xx devices (note, two NUCLEO-WL55JC1 are needed; for the Tx and Rx, respectively). The Tx device transmits a short 10 byte packet every 2s, and the Rx device receives these packets and displays the payload on the hyperterminal. 
   * The example also aims to demonstrate to the user how to configure the system (pinout, clocks, peripherals, utilities) and SubGhz middleware from STM32CubeMX to enable the radio in FSK modulation mode, and show how to schedule and enable a transmission and receive operation from the firmware application.       
   * Development tools, toolchains/compilers: STM32CubeMX v6.5.0, IAR EWARM V9.20.4, STM32CubeIDE v1.8.0
   * Supported Devices and hardware boards: NUCLEO-WL55JC1
   * Known limitations: None
   
## Boards Needed

  * Two NUCLEO-WL55JC1
    * [NUCLEO-WL55JC](https://www.st.com/en/evaluation-tools/nucleo-wl55jc.html)

## Troubleshooting

Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) guide.
