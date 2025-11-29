add_library(
  STM32F4_HAL
  stm32f4xx_hal_driver/Src/stm32f4xx_hal.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_cortex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_dma.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_dma_ex.c
  stm32f4xx_hal_driver/Src/Legacy/stm32f4xx_hal_eth.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ramfunc.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_gpio.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_hcd.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_iwdg.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_pcd.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_pcd_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_pwr.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_pwr_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_rng.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_rtc.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_rtc_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_spi.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim_ex.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_uart.c
  stm32f4xx_hal_driver/Src/stm32f4xx_hal_wwdg.c
  stm32f4xx_hal_driver/Src/stm32f4xx_ll_usb.c
  )

target_include_directories(STM32F4_HAL PUBLIC stm32f4xx_hal_driver/Inc)
target_include_directories(STM32F4_HAL PUBLIC stm32f4xx_hal_driver/Inc/Legacy)
target_link_libraries(STM32F4_HAL PUBLIC STM32F4_HAL_Config STM32F4xx::CMSIS)

add_library(STM32F4::HAL ALIAS STM32F4_HAL)
