################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Services/esp32_service.c \
../User/Services/sensor_service.c 

OBJS += \
./User/Services/esp32_service.o \
./User/Services/sensor_service.o 

C_DEPS += \
./User/Services/esp32_service.d \
./User/Services/sensor_service.d 


# Each subdirectory must supply rules for building sources it contributes
User/Services/%.o User/Services/%.su User/Services/%.cyclo: ../User/Services/%.c User/Services/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I"C:/Users/ADMIN/Documents/Code/BTL_TK_Hethongnhung/User/Drivers/UGUI" -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/ADMIN/Documents/Code/BTL_TK_Hethongnhung/User/App" -I"C:/Users/ADMIN/Documents/Code/BTL_TK_Hethongnhung/User/Services" -I"C:/Users/ADMIN/Documents/Code/BTL_TK_Hethongnhung/User/Drivers" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Services

clean-User-2f-Services:
	-$(RM) ./User/Services/esp32_service.cyclo ./User/Services/esp32_service.d ./User/Services/esp32_service.o ./User/Services/esp32_service.su ./User/Services/sensor_service.cyclo ./User/Services/sensor_service.d ./User/Services/sensor_service.o ./User/Services/sensor_service.su

.PHONY: clean-User-2f-Services

