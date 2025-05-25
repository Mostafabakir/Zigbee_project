################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_entry.c \
../Core/Src/main.c \
../Core/Src/stm32wbaxx_hal_msp.c \
../Core/Src/stm32wbaxx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32wbaxx.c 

OBJS += \
./Core/Src/app_entry.o \
./Core/Src/main.o \
./Core/Src/stm32wbaxx_hal_msp.o \
./Core/Src/stm32wbaxx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32wbaxx.o 

C_DEPS += \
./Core/Src/app_entry.d \
./Core/Src/main.d \
./Core/Src/stm32wbaxx_hal_msp.d \
./Core/Src/stm32wbaxx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32wbaxx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DMAC -DMAC_LAYER -DDISABLE_RFTS_EXT_EVNT_HNDLR=1 -DUSE_HAL_DRIVER -DSTM32WBA55xx -c -I../Core/Inc -I../System/Interfaces -I../System/Config/Log -I../System/Config/LowPower -I../System/Config/Debug_GPIO -I../STM32_WPAN/Target -I../STM32_WPAN/App -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/trace/adv_trace -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/STM32WBAxx_HAL_Driver/Inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/STM32WBAxx_HAL_Driver/Inc/Legacy -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Interfaces -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/BasicAES -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/MemoryManager -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/RTDebug -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/SerialCmdInterpreter -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/Log -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/misc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/sequencer -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/tim_serv -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/lpm/tiny_lpm -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/config/ieee_15_4_basic -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/CMSIS/Device/ST/STM32WBAxx/Include -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc/_40nm_reg_files -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc/ot_inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_sys/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/general -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/key -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/se -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/security -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/zd -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zgp -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/stack/port/mac -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/stack/port/stm32wba -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/CMSIS/Include -I"E:/Freelancing projects/software_part/project/newdev/Zigbee_Messaging_Server_Router/Drivers/BSP/STM32WBAxx_Nucleo" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_entry.cyclo ./Core/Src/app_entry.d ./Core/Src/app_entry.o ./Core/Src/app_entry.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32wbaxx_hal_msp.cyclo ./Core/Src/stm32wbaxx_hal_msp.d ./Core/Src/stm32wbaxx_hal_msp.o ./Core/Src/stm32wbaxx_hal_msp.su ./Core/Src/stm32wbaxx_it.cyclo ./Core/Src/stm32wbaxx_it.d ./Core/Src/stm32wbaxx_it.o ./Core/Src/stm32wbaxx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32wbaxx.cyclo ./Core/Src/system_stm32wbaxx.d ./Core/Src/system_stm32wbaxx.o ./Core/Src/system_stm32wbaxx.su

.PHONY: clean-Core-2f-Src

