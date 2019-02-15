################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/eth.c \
../Src/gpio.c \
../Src/io.c \
../Src/main.c \
../Src/sdio_h7xx.c \
../Src/sdmmc.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_it.c \
../Src/system_stm32h7xx.c \
../Src/unity.c \
../Src/usart.c \
../Src/usb_device.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c \
../Src/usbd_storage_if.c 

OBJS += \
./Src/eth.o \
./Src/gpio.o \
./Src/io.o \
./Src/main.o \
./Src/sdio_h7xx.o \
./Src/sdmmc.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_it.o \
./Src/system_stm32h7xx.o \
./Src/unity.o \
./Src/usart.o \
./Src/usb_device.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o \
./Src/usbd_storage_if.o 

C_DEPS += \
./Src/eth.d \
./Src/gpio.d \
./Src/io.d \
./Src/main.d \
./Src/sdio_h7xx.d \
./Src/sdmmc.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_it.d \
./Src/system_stm32h7xx.d \
./Src/unity.d \
./Src/usart.d \
./Src/usb_device.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d \
./Src/usbd_storage_if.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' -DUNITY_INCLUDE_CONFIG_H -DSTM32H7 '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"/Users/khockuba/STM32/h7 test/Inc" -I"/Users/khockuba/STM32/h7 test/Drivers/STM32H7xx_HAL_Driver/Inc" -I"/Users/khockuba/STM32/h7 test/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"/Users/khockuba/STM32/h7 test/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"/Users/khockuba/STM32/h7 test/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"/Users/khockuba/STM32/h7 test/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"/Users/khockuba/STM32/h7 test/Drivers/CMSIS/Include"  -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


