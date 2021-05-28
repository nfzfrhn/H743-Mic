################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP_Custom/H743_audio_ex.c 

OBJS += \
./Drivers/BSP_Custom/H743_audio_ex.o 

C_DEPS += \
./Drivers/BSP_Custom/H743_audio_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP_Custom/H743_audio_ex.o: ../Drivers/BSP_Custom/H743_audio_ex.c Drivers/BSP_Custom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I"C:/Users/Nafiz/STM32CubeIDE/workspace_1.0.2/H743-Mic2/Application" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc -I"C:/Users/Nafiz/STM32CubeIDE/workspace_1.0.2/H743-Mic2/Application/DriverEx" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/BSP_Custom/H743_audio_ex.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

