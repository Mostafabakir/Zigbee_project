################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../STM32_WPAN/App/app_bsp.c \
../STM32_WPAN/App/app_zigbee.c \
../STM32_WPAN/App/app_zigbee_endpoint.c \
../STM32_WPAN/App/esl_cluster.c \
../STM32_WPAN/App/esl_cluster_helpers.c 

OBJS += \
./STM32_WPAN/App/app_bsp.o \
./STM32_WPAN/App/app_zigbee.o \
./STM32_WPAN/App/app_zigbee_endpoint.o \
./STM32_WPAN/App/esl_cluster.o \
./STM32_WPAN/App/esl_cluster_helpers.o 

C_DEPS += \
./STM32_WPAN/App/app_bsp.d \
./STM32_WPAN/App/app_zigbee.d \
./STM32_WPAN/App/app_zigbee_endpoint.d \
./STM32_WPAN/App/esl_cluster.d \
./STM32_WPAN/App/esl_cluster_helpers.d 


# Each subdirectory must supply rules for building sources it contributes
STM32_WPAN/App/%.o STM32_WPAN/App/%.su STM32_WPAN/App/%.cyclo: ../STM32_WPAN/App/%.c STM32_WPAN/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DMAC -DMAC_LAYER -DDISABLE_RFTS_EXT_EVNT_HNDLR=1 -DUSE_HAL_DRIVER -DSTM32WBA55xx -c -I../Core/Inc -I../System/Interfaces -I../System/Config/Log -I../System/Config/LowPower -I../System/Config/Debug_GPIO -I../STM32_WPAN/Target -I../STM32_WPAN/App -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/trace/adv_trace -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/STM32WBAxx_HAL_Driver/Inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/STM32WBAxx_HAL_Driver/Inc/Legacy -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Interfaces -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/BasicAES -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/MemoryManager -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/RTDebug -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/SerialCmdInterpreter -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Projects/Common/WPAN/Modules/Log -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/misc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/sequencer -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/tim_serv -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Utilities/lpm/tiny_lpm -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/config/ieee_15_4_basic -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/CMSIS/Device/ST/STM32WBAxx/Include -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc/_40nm_reg_files -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/inc/ot_inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/link_layer/ll_sys/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/mac_802_15_4/core/inc -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/general -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/key -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/se -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/security -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zcl/zd -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/include/zgp -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/stack/port/mac -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Middlewares/ST/STM32_WPAN/zigbee/stack/port/stm32wba -IC:/Users/2024/STM32Cube/Repository/STM32Cube_FW_WBA_V1.6.1/Drivers/CMSIS/Include -I"E:/Freelancing projects/software_part/project/newdev/Zigbee_Messaging_Server_Router/Drivers/BSP/STM32WBAxx_Nucleo" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-STM32_WPAN-2f-App

clean-STM32_WPAN-2f-App:
	-$(RM) ./STM32_WPAN/App/app_bsp.cyclo ./STM32_WPAN/App/app_bsp.d ./STM32_WPAN/App/app_bsp.o ./STM32_WPAN/App/app_bsp.su ./STM32_WPAN/App/app_zigbee.cyclo ./STM32_WPAN/App/app_zigbee.d ./STM32_WPAN/App/app_zigbee.o ./STM32_WPAN/App/app_zigbee.su ./STM32_WPAN/App/app_zigbee_endpoint.cyclo ./STM32_WPAN/App/app_zigbee_endpoint.d ./STM32_WPAN/App/app_zigbee_endpoint.o ./STM32_WPAN/App/app_zigbee_endpoint.su ./STM32_WPAN/App/esl_cluster.cyclo ./STM32_WPAN/App/esl_cluster.d ./STM32_WPAN/App/esl_cluster.o ./STM32_WPAN/App/esl_cluster.su ./STM32_WPAN/App/esl_cluster_helpers.cyclo ./STM32_WPAN/App/esl_cluster_helpers.d ./STM32_WPAN/App/esl_cluster_helpers.o ./STM32_WPAN/App/esl_cluster_helpers.su

.PHONY: clean-STM32_WPAN-2f-App

